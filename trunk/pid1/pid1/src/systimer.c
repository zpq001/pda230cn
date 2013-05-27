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
uint8_t autoPowerOffState = 0;

SoftTimer8b_t menuUpdateTimer = {
	.Enabled = 1,
	.RunOnce = 0,
	.CompA = 0,
	.FOvfl = 0,
	.FA_TGL = 0,
	.Timer = 0,
	.Top = MENU_UPDATE_INTERVAL
};	

SoftTimer8b_t systickTimer = {
	.Enabled = 1,
	.RunOnce = 0,
	.Timer = 0,
	.Top = SYSTICKS_PER_SECOND - 1
};	

SoftTimer8b_t secondsTimer = {
	.Enabled = 1,
	.RunOnce = 0,
	.Timer = 0,
	.Top = 60 - 1
};	

SoftTimer8b_t minutesTimer = {
	.Enabled = 1,
	.RunOnce = 0,
	.Timer = 0,
	.Top = MAX_POWEROFF_TIMEOUT - 1;
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





void resetAutoPowerOffCounter(void)
{
	autoPowerOffState |= RESET_COUNTER;
}

void processAutoPowerOff(void)
{
	minutesTimer.CompA = power_off_timeout;
	switch(autoPowerOffState & 0x0F)
	{
		case 0:
			if (autoPowerOffState & RESET_COUNTER)
			{
				minutesTimer = 0;
				minutesTimer.FA_GE = 0;
				autoPowerOffState = 0;
			}
			else if (minutesTimer.FA_GE)
			{
				autoPowerOffState = AUTO_POFF_ENTER;
			}
			break;
		case AUTO_POFF_ENTER:
			autoPowerOffState = AUTO_POFF_ACTIVE;
			break;
		case AUTO_POFF_ACTIVE:
			if (autoPowerOffState & RESET_COUNTER)
			{
				minutesTimer = 0;
				minutesTimer.FA_GE = 0;
				autoPowerOffState = AUTO_POFF_LEAVE;
			}
			break;
		case AUTO_POFF_LEAVE:
			autoPowerOffState = 0;
			break;
	}
}


ISR(TIMER2_COMP_vect)
{	
	static uint16_t poff_timer_low = 0;
	static uint8_t poff_timer_high = 0;
	
	// Manage beeper
	if (beep_cnt)
		beep_cnt--;
	else
		SetBeepOutput(0);	// done
	
	// Manage LED indicator
	processLedIndicator();
	
	// Process menu update timer
	processSoftTimer8b(&menuUpdateTimer);	
	
	// Process time
	if (menuUpdateTimer.FOvfl)
	{
		processSoftTimer8b(&systickTimer);
		if (systickTimer.FOvfl)
		{
			processSoftTimer8b(&secondsTimer);
			if (secondsTimer.FOvfl)
			{
				processSoftTimer8b(&minutesTimer);
			}
		
		}
	}
}


