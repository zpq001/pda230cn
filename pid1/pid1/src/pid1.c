/*
 * pid1.c
 *
 * Created: 24.03.2013 1:35:17
 *  Author: Avega
 
 Asynchronous operations:
	ISR:	Analog comparator (AC line sync)
	ISR:	Timer0 (used for power control)
	ISR:	Timer2 (system timer)
	ISR:	ADC (ADC conversion is started by Timer0)
 
 */ 



//#include <avr/io.h>
//#include <util/setbaud.h>
//#include <avr/interrupt.h>

//#include <stdlib.h>
//#include <stdio.h>

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

extern volatile SoftTimer8b_t menuUpdateTimer;


////// debug ///////
//extern void powTest(void);

void init_system_io()
{
	// Setup Port D
	PORTD = 0; //(1<<PD_SYNCA | 1<<PD_SYNCB);
	DDRD = (1<<PD_TXD | 1<<PD_M1 | 1<<PD_M2 | 1<<PD_HEATER | 1<<PD_HEAT_INDIC );
	
	// Setup Port B
	PORTB = 0;
	DDRB = (1<<PB_BEEPER | 1<<PB_LED_DOUT | 1<<PB_LED_CLK | 1<<PB_SEGF | 1<<PB_SEGG | 1<<PB_SEGH);
	
	PORTC = 0;
	DDRC = (1<<PC_SEGA | 1<<PC_SEGB | 1<<PC_SEGC | 1<<PC_SEGD | 1<<PC_SEGE);	

	// Setup timer0
	// 1/1024 prescaler, T=64us @16MHz
	TCCR0 = (1<<CS02 | 0<<CS01 | 1<<CS00);
	// Start 256 * 64us = 16384us interval
	TCNT0 = 0;
	// Clear interrupt flag
	TIFR |= (1<<TOV0);
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
	
	// Setup timer2 as 1ms system frequency generator
	// 1/64 prescaler, CTC mode, OC2 disconnected
	TCCR2 = (1<<CS22 | 1<<WGM21);
	OCR2 = 249;
	TIMSK |= (1<<OCIE2);
	
	// Setup timer1 as beeper frequency generator
	// CTC mode, 1/64 prescaler
	TCCR1A = 0;
	TCCR1B = (1<<WGM12 | 1<<CS11 | 1<<CS10);
	// 1kHz beeper frequency default
	OCR1A = 127;

	// Setup AC sync comparator
	// Interrupt on output toggle
	ACSR |= (1<<ACI);
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
	// 38400 @16MHz, 2x
//	UBRRH=0x00;
//	UBRRL=0x33;
	// 19200 @16MHz, 2x
//	UBRRH=0x00;
//	UBRRL=0x67;

	
}




int main(void)
{
	char str[10];
	volatile uint8_t temp8u = 0x00;
	volatile uint16_t temp16u;
	
	// Initialize IO
	init_system_io();
	// Restore params
	restoreGlobalParams();
	// Calibrate ADC coefficients using restored params
	calculateCoeffs();
	// Initialize LED indicator
	initLedIndicator();
	// Initialize menu
	InitMenu();
	// Enable interrupts
	sei();
	// Beep
	SetBeeperFreq(1000);
	StartBeep(200);
	// Safety delay for power part
	_delay_ms(100);
	// Start rotating
	setMotorDirection(ROLL_FWD);


    while(1)
    {
		
		if (menuUpdateTimer.FOvfl)
		{
			// Get new temperature measurement - new value is pushed into ring buffer
			// once every AC line period
			update_normalized_adc();			// TODO: slow down temperature change (say once per 200-400ms)
			
			// Get new button state
			process_buttons();
			
			// Give sound feedback
			if (button_state & BL_MENU)
			{
				SetBeeperFreq(800);
				StartBeep(40);
			}
			else if (button_action_down & (BD_MENU | BD_UP | BD_DOWN | BD_HEATCTRL))
			{
				SetBeeperFreq(1000);
				StartBeep(40);
			}	
			

			// Process user menu states, settings and indication
			processMenu();
			
			// Process cyclic rolling, direction control
			processRollControl();	
			
			// Process heater regulation
			processHeaterControl();
		

			// Process log
			if (menuUpdateTimer.FA_TGL)
			{
				//---------------------------------//
				// Log to UART
				//---------------------------------//
				// Function is called every 50ms
				// UART message is sent every second call (once per 100ms)
				
				u16toa_align_right(adc_celsius,str,6,' ');				// Displayed temp, Celsius
				USART_sendstr(str);
				
				u16toa_align_right(adc_normalized,str,6,' ');			// Displayed temp
				USART_sendstr(str);
				
				u16toa_align_right(dbg_SetTempCelsius,str,6,' ');		// Temp setting, Celsius
				USART_sendstr(str);
				
				u16toa_align_right(dbg_SetTempPID,str,6,' ');			// Temp setting, as input to PID
				USART_sendstr(str);
				
				u16toa_align_right(dbg_RealTempCelsius,str,8,' ');		// Real temp, sampled for PID input, Celsius
				USART_sendstr(str);
				
				u16toa_align_right(dbg_RealTempPID,str,6,' ');			// Real temp, sampled for PID input
				USART_sendstr(str);
				
				//u16toa_align_right(dbg_RealTempPIDfiltered,str,6,' ');			// Real temp, sampled for PID input, filtered
				//USART_sendstr(str);
				
				USART_sendstr("    ");
				
				if (dbg_PID_p_term >= 0)
				{
					u16toa_align_right(dbg_PID_p_term,str,6,'0');		// p term
					USART_sendstr(str);	
				}
				else
				{
					u16toa_align_right(-dbg_PID_p_term,str,6,'0');		
					USART_send('-');
					USART_sendstr(str);
				}
				
				USART_sendstr("    ");
				
				if (dbg_PID_d_term >= 0)
				{
					u16toa_align_right(dbg_PID_d_term,str,6,'0');		// d term
					USART_sendstr(str);
				}
				else
				{
					u16toa_align_right(-dbg_PID_d_term,str,6,'0');
					USART_send('-');
					USART_sendstr(str);
				}
				
				USART_sendstr("    ");
				
				if (dbg_PID_i_term >= 0)
				{
					u16toa_align_right(dbg_PID_i_term,str,6,'0');		// i term
					USART_sendstr(str);
				}
				else
				{
					u16toa_align_right(-dbg_PID_i_term,str,6,'0');
					USART_send('-');
					USART_sendstr(str);
				}
				
				u16toa_align_right(dbg_PID_output,str,6,' ');			// PID output
				USART_sendstr(str);
				
				
				u16toa_align_right(ctrl_heater,str,6,' ');				// Heater control (PID output, synchronized)
				USART_sendstr(str);
				
				
				USART_sendstr("\n\r");
				
				

				//---------------------------------//
				
			}
			
			
			processAutoPowerOff();	// TODO
			
			
			
			menuUpdateTimer.FOvfl = 0;	
		}
		
    }
}