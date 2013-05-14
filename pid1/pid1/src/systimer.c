/*
 * systimer.c
 *
 * Created: 24.03.2013 14:58:22
 *  Author: Avega
 */ 


#include <avr/interrupt.h>
#include "soft_timer.h"
#include "systimer.h"
#include "led_indic.h"
#include "adc.h"
#include "control.h"



static uint16_t beep_cnt = 0;

SoftTimer8b_t menuUpdateTimer = {
	.Enabled = 1,
	.RunOnce = 0,
	.CompA = 0,
	.FOvfl = 0,
	.FA_TGL = 0,
	.Timer = 0,
	.Top = MENU_UPDATE_INTERVAL
};	
	


// Enable / disable beeper output
inline void SetBeepOutput(uint8_t val)
{
	if (val)
		// Toggle OCR1A on compare match
		TCCR1A |= (1<<COM1A0);
	else
		// Disable OCR1A output
		TCCR1A &= ~(1<<COM1A0 | 1<<COM1A1);
}

// Setup beeper period 
void SetBeeperPeriod(uint16_t new_period_us)
{
	// Timer1 runs at 250kHz, T = 4us
	// Output toggles on every compare match
	if (new_period_us & 0xFFF8)
		OCR1A = (new_period_us>>3) - 1;
	else
		OCR1A = 0x1;
	TCNT1 = 0;
}

// Setup beeper frequency (Hz)
void SetBeeperFreq(uint16_t freq_hz)
{
	uint16_t period_us = 1000000 / freq_hz;
	if (period_us & 0xFFF8)
		OCR1A = (period_us>>3) - 1;
	else
		OCR1A = 0x1;
	TCNT1 = 0;
}

// Beep for some time in ms
void StartBeep(uint16_t time_ms)
{
	if (sound_enable)
	{
		beep_cnt = time_ms;
		SetBeepOutput(1);		
	}
}

// Stop beeper
void StopBeep()
{
	beep_cnt = 0;
	SetBeepOutput(0);
}



ISR(TIMER2_COMP_vect)
{	
	// Manage beeper
	if (beep_cnt)
		beep_cnt--;
	else
		SetBeepOutput(0);	// done
	
	// Manage LED indicator
	processLedIndicator();
	
	// Process menu update timer
	processSoftTimer8b(&menuUpdateTimer);	
	
}


