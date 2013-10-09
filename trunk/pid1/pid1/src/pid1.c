/*
 * pid1.c
 *
 * Created: 24.03.2013 1:35:17
 *  Author: Avega
 
 Asynchronous operations:
	ISR:	Analog comparator (AC line sync)
	ISR:	Timer0 (used for power control)
	ISR:	Timer2 (system timer)
	ISR:	ADC (ADC conversion is started by Timer2 ISR)
 
 */ 



#include "compilers.h"
#include "soft_timer.h"	
#include "systimer.h"	
#include "port_defs.h"
#include "led_indic.h"
#include "buttons.h"
#include "power_control.h"
#include "adc.h"
#include "usart.h"
#include "control.h"
#include "menu.h"
#include "my_string.h"
#include "pid_controller.h"

extern volatile SoftTimer8b_t menuUpdateTimer;	// Must be declared volatile here

// Functions for log optimization
static void logU16p(uint16_t val);
static void logI32p(int32_t val);


static void init_system_io()
{
	// Setup Port D
	PORTD = 0;//(1<<PD_SYNCA | 1<<PD_SYNCB);
	DDRD = (1<<PD_TXD | 1<<PD_M1 | 1<<PD_M2 | 1<<PD_HEATER | 1<<PD_HEAT_INDIC );
	
	// Setup Port B
	PORTB = 0;
	DDRB = (1<<PB_BEEPER | 1<<PB_LED_DOUT | 1<<PB_LED_CLK | 1<<PB_SEGF | 1<<PB_SEGG | 1<<PB_SEGH);
	
	PORTC = 0;
	DDRC = (1<<PC_SEGA | 1<<PC_SEGB | 1<<PC_SEGC | 1<<PC_SEGD | 1<<PC_SEGE);	

	// Setup timer0
	// 1/1024 prescaler, T=64us @16MHz
	TCCR0 = (1<<CS02 | 0<<CS01 | 1<<CS00);
	TCNT0 = 0;
	// Clear interrupt flag
	TIFR = (1<<TOV0);
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
	
	// Setup timer2 as 1ms system frequency generator
	// 1/64 prescaler, CTC mode, OC2 disconnected
	TCCR2 = (1<<CS22 | 1<<WGM21);
	OCR2 = 249;						// 1ms @ 16MHz
	TIMSK |= (1<<OCIE2);
	
	// Setup timer1 as beeper frequency generator
	// CTC mode, 1/64 prescaler
	TCCR1A = 0;
	TCCR1B = (1<<WGM12 | 1<<CS11 | 1<<CS10);
	// 1kHz beeper frequency default
	OCR1A = 127;

	// Setup AC sync comparator
	// Interrupt on output toggle
	ACSR = (1<<ACIE | 0<<ACIS1 | 0<<ACIS0);
	
	// Setup ADC
	// Internal Vref + cap, input ADC5, 
	ADMUX = (1<<REFS1 | 1<<REFS0 | 0<<MUX3 | 1<<MUX2 | 0<<MUX1 | 1<<MUX0 );
	// Prescaler = 128
	ADCSRA = (1<<ADEN |/* 1<<ADFR |*/  1<<ADIE | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0);
	
	// Setup USART
	// Double speed
	UCSRA = (1<<U2X);
	UCSRB = (1<<RXEN | 1<<TXEN | 0<<UCSZ2);
	// No parity, 1 stop bit, 8 bit
	UCSRC = (1<<URSEL | 0<<UPM1 | 0<<UPM0 | 0<<USBS | 1<<UCSZ1 | 1<<UCSZ0);
	// 57600 @16MHz, 2x
	UBRRH=0x00;
	UBRRL=0x22;
	
}


char str[20];

int main(void)
{
	volatile uint8_t temp8u = 0x00;
	volatile uint16_t temp16u;
	// Initialize MCU IO
	init_system_io();
	// Restore params from EEPROM
	// If some values are corrupted, settings or/and calibration are loaded with default configuration.
	temp8u = restoreGlobalParams();
	// Calibrate ADC coefficients using restored params
	calculateCoeffs();
	// Initialize LED indicator
	initLedIndicator();
	// Clear comparator interrupt flag to prevent false triggering
	ACSR |= (1<<ACI);
	sei();
	// If default values were loaded from EEPROM, inform user
	#ifdef USE_EEPROM_CRC
	if (temp8u)
	{
		printLedBuffer(0,"ERR E");
		fillLedBuffer(5,1,temp8u + 0x30);	// error code: E1 - CRC error in global params, E2 - CRC error in calibration, E3 - both
		Sound_Play(m_beep_err1);
		_delay_ms(1000);
	} 
	#endif
	// Dump calibration data over UART - might be useful for debug
	logU16p(cp.cpoint1);
	logU16p(cp.cpoint1_adc);
	logU16p(cp.cpoint2);
	logU16p(cp.cpoint2_adc);
	USART_sendstr("\n\r");
	// Safety delay for power part and ADC buffer
	_delay_ms(100);
	// Check AC line
	if(p_state == 0x0F) 	
	{
		// Power control state machine has not changed - sync has not been detected
		printLedBuffer(0,"ERR AC");
		_delay_ms(1000);
	}
	// Initialize menu
	InitMenu();
	// Beep
	Sound_Play(m_beep_1000Hz_100ms);
	
	// When we get here, full ADC buffer have been sampled
	// Get oversampled and filtered ADC for PID controller
	// update_normalized_adc();	// not required here
	
	// Start rotating
	setMotorDirection(ROLL_FWD);
	// Clear timer
	menuUpdateTimer.FOvfl = 0;
	// Enable watchdog
	wdt_enable(WDTO_1S);

    while(1)
    {
		if (menuUpdateTimer.FOvfl)
		{
			// Reset watchdog timer
			wdt_reset();
			
			//--------- BUTTONS ----------//
			
			// Get new button state
			process_buttons();
			
			// Give sound feedback
			if (button_state & (BL_MENU | BL_HEATCTRL))
			{
				Sound_Play(m_beep_800Hz_40ms);
			}
			else if (button_action_down & (BD_MENU | BD_UP | BD_DOWN | BD_HEATCTRL))
			{
				Sound_Play(m_beep_1000Hz_40ms);
			}	
			// Direction control buttons sounds get special processing at processRollControl()
			
			// If any button is pressed, restart power off interval
			if (button_action_down)
				resetAutoPowerOffCounter();
			
			//---------- TIMERS ----------//
			
			// Process timers and time counters - it is important to call
			// this function before menu and power controls processing
			processSystemTimers();

			// Warn user about soon auto powering off
			if (sys_timers.flags & AUTOPOFF_SOON)
			{
				Sound_Play(m_beep_warn_poff);
			}		
			
			//----------- ADC ------------//
			
			// Get new temperature measurement
			update_normalized_adc();			
		
			// Update indicated Celsius degree
			if (sys_timers.flags & EXPIRED_CELSIUS)
				update_Celsius();		
			
			//----------- MENU -----------//		
				
			// Process user menu states, settings and indication
			processMenu();
			
			//--------- CONTROLS ---------//		
			
			// Process cyclic rolling, direction control
			processRollControl();	
			
			// Process heater events monitoring
			// Must be called before processHeaterControl()
			processHeaterEvents();
			
			// Process heater regulation
			processHeaterControl();
		
			// Process heater warnings
			processHeaterAlerts();
			
			
			
			//------- LOG to UART --------//	
			if (sys_timers.flags & EXPIRED_LOG)
			{
				
				logU16p(adc_celsius);					// Actual temp Celsius
				logU16p(adc_normalized);				// Actual temp (ADC), normalized
				logU16p(adc_filtered);					// Actual temp (ADC), oversampled, filtered
				USART_sendstr("    ");
				
				logU16p(dbg_SetPointPID);				// PID setpoint
				logU16p(dbg_RealTempPID);				// PID process value
				
				logI32p(dbg_PID_p_term);				// p term
				logI32p(dbg_PID_d_term);				// d term
				logI32p(dbg_PID_i_term);				// i term
				USART_sendstr("    ");
				
				logU16p(dbg_PID_output);				// PID output
			
				USART_sendstr("\n\r");

				//---------------------------------//
				
			}
			
			
			menuUpdateTimer.FOvfl = 0;	
		}
		
    }
}


static void logU16p(uint16_t val)
{
	u16toa_align_right(val,str,6);
	USART_sendstr(str);
}

static void logI32p(int32_t val)
{
	i32toa_align_right(val,str,12);
	USART_sendstr(str);
}




