/*
 * led_indic.c
 *
 * Created: 24.03.2013 18:23:11
 *  Author: Avega
 */ 


#include "compilers.h"
#include "led_indic.h"
#include "port_defs.h"


// Clocks in bit into the shift register
void led_clock_pulse(uint8_t bit)
{
	if (bit)
		PORTB |= (1<<PB_LED_DOUT);
	else
		PORTB &= ~(1<<PB_LED_DOUT);
	_delay_us(SETUP_DELAY_US);
	PORTB |= (1<<PB_LED_CLK);
	_delay_us(SHIFT_DELAY_US);
	PORTB &= ~(1<<PB_LED_CLK);
}

// Outputs decoded segment data to port bits
void set_led_segments(uint8_t seg_ch)
{
	uint8_t pdat;
	seg_ch ^= LED_SEGMENT_MASK;
	pdat = PORTC & ~( (1<<PC_SEGA) | (1<<PC_SEGB) | (1<<PC_SEGC) | (1<<PC_SEGD) | (1<<PC_SEGE) );
	pdat |= (seg_ch & ( (1<<PC_SEGA) | (1<<PC_SEGB) | (1<<PC_SEGC) | (1<<PC_SEGD) | (1<<PC_SEGE) ));
	PORTC = pdat;
	seg_ch >>= 2;
	pdat = PORTB & ~( (1<<PB_SEGF) | (1<<PB_SEGG) | (1<<PB_SEGH) );
	pdat |= (seg_ch & ( (1<<PB_SEGF) | (1<<PB_SEGG) | (1<<PB_SEGH) ));
	PORTB = pdat;
}		

// Turns segment port bits to HI-Z and
// enables pull-ups
void enable_led_segments_pullups()
{
	PORTC |= ( (1<<PC_SEGA) | (1<<PC_SEGB) | (1<<PC_SEGC) | (1<<PC_SEGD) | (1<<PC_SEGE) );
	PORTB |= ( (1<<PB_SEGF) | (1<<PB_SEGG) | (1<<PB_SEGH) );
}	

void enable_led_segments()
{
	DDRC |= ( (1<<PC_SEGA) | (1<<PC_SEGB) | (1<<PC_SEGC) | (1<<PC_SEGD) | (1<<PC_SEGE) );
	DDRB |= ( (1<<PB_SEGF) | (1<<PB_SEGG) | (1<<PB_SEGH) );
}

void disable_led_segments()
{
	DDRC &= ~( (1<<PC_SEGA) | (1<<PC_SEGB) | (1<<PC_SEGC) | (1<<PC_SEGD) | (1<<PC_SEGE) );
	DDRB &= ~( (1<<PB_SEGF) | (1<<PB_SEGG) | (1<<PB_SEGH) );
}	


void capture_button_state()
{
	uint8_t pdat;
	pdat = PINB  & ( (1<<PB_SEGF) | (1<<PB_SEGG) | (1<<PB_SEGH) );
	pdat <<= 2;
	pdat |= (PINC & ( (1<<PC_SEGA) | (1<<PC_SEGB) | (1<<PC_SEGC) | (1<<PC_SEGD) | (1<<PC_SEGE) ));
	raw_button_state = pdat ^ RAW_BUTTONS_INVERSE_MASK;
}







