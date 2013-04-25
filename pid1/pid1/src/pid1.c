/*
 * pid1.c
 *
 * Created: 24.03.2013 1:35:17
 *  Author: Avega
 
 Asynchronious operations:
	ISR:	Analog comparator (AC line sync)
	ISR:	Timer0 (used for power control)
	ISR:	Timer2 (system timer)
	ISR:	ADC (ADC conversion is started by system timer)
	
 
 
 
 
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


void init_system()
{
	// Setup Port D
	PORTD = 0; //(1<<PD_SYNCA | 1<<PD_SYNCB);
	DDRD = (1<<PD5 | 1<<PD_TXD | 1<<PD_M1 | 1<<PD_M2 | 1<<PD_HEATER );
	
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
	uint8_t temp8u = 0x00;
	uint8_t uart_log_timeout_counter = 0;
	uint16_t uart_log_counter = 0;
	
	// Initialize IO
	init_system();
	// Initialize LED indicator
	initLedIndicator();
	// Enable interrupts
	sei();
	
	// Beep
	SetBeeperFreq(1000);
	StartBeep(100);
	
	InitMenu();

	setMotorDirection(ROTATE_FORWARD);
	
	//button_state = BD_UP;
	//processMenu();
	
	printLedBuffer(0,"      ");
	
    while(1)
    {
		
		if (menuUpdateTimer.FOvfl)
		{
			// Get new temperature measurement
			update_filtered_adc();
			
			// Get new button state
			process_buttons();
			
			// Give audio feedback
			if (sound_enable)
			{
				/*
				if (button_signal & LONG_PRESS_EVENT)
				{
					if (raw_button_state & BD_MENU)
					{
						SetBeeperFreq(800);
						StartBeep(50);
					}
				}
				*/
				if (button_state & BL_MENU)
				{
					SetBeeperFreq(800);
					StartBeep(40);
				}
				else if (button_action_down)
				{
					SetBeeperFreq(1000);
					StartBeep(40);
				}	
			}
			

			// Process user menu states, settings and indication
			processMenu();
			
			// Process cyclic rolling, direction control
			processRollControl();	
			
			// Process heater regulation
			processHeaterControl();
		


			if (menuUpdateTimer.FA_TGL)
			{
				//---------------------------------//
				// Log to UART
				//---------------------------------//
				// Function is called every 100ms
				// UART message is sent every second
				if (uart_log_timeout_counter == 9)
				{
					uart_log_timeout_counter = 0;
					
					
					u16toa_align_right(uart_log_counter,str,5,' ');
					USART_sendstr(str);
					USART_sendstr("     ");
					
					u16toa_align_right(adc_filtered_value,str,5,' ');
					USART_sendstr(str);
					USART_sendstr("     ");
					
					if (p_flags & HEATER_ENABLED)
					USART_send('1');
					else
					USART_send('0');
					
					USART_sendstr("\n\r");
					
					uart_log_counter++;
					
				}
				else
				{
					uart_log_timeout_counter++;
				}
				//---------------------------------//
				
			}
			
			
			
			
			menuUpdateTimer.FOvfl = 0;	
		}
		
    }
}