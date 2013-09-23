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
static uint8_t enableOverride = 0;


SoftTimer8b_t menuUpdateTimer = {
	.Enabled = 1,
	.RunOnce = 0,
	.CompA = 0,
	.FOvfl = 0,
	.FA_TGL = 0,
	.Timer = 0,
	.Top = MENU_UPDATE_INTERVAL
};	


sys_timers_t sys_timers = {
	.celsius_upd_counter = 1,					// To force Celsius update  - timer is decremental
	.log_counter = 1,							// To force log message		- timer is decremental
	.counter_10sec = COUNTER_10SEC_INTERVAL, 	//							- timer is decremental
	.counter_1min = COUNTER_1MIN_INTERVAL,		//							- timer is decremental
	.poff_counter = 0,							//							- timer is incremental
	.pid_update_counter = PID_UPDATE_INTERVAL	//							- timer is decremental
};



void processSystemTimers(void)
{
	sys_timers.flags = 0x00;
	
	// Process Celsius counter
	if (--sys_timers.celsius_upd_counter == 0)
	{
		sys_timers.celsius_upd_counter = CELSIUS_UDPATE_INTERVAL;
		sys_timers.flags |= EXPIRED_CELSIUS;
		
		// Process PID update counter
		if (--sys_timers.pid_update_counter == 0)
		{
			sys_timers.pid_update_counter = PID_UPDATE_INTERVAL;
			sys_timers.flags |= UPDATE_PID;
		}
	}
	
	// Process log counter
	if (--sys_timers.log_counter == 0)
	{
		sys_timers.log_counter = LOG_INTERVAL;
		sys_timers.flags |= EXPIRED_LOG;
	}
	
	// Process 10 seconds counter
	if (--sys_timers.counter_10sec == 0)
	{
		sys_timers.counter_10sec = COUNTER_10SEC_INTERVAL;
		sys_timers.flags |= EXPIRED_10SEC;
		
		// Process 1 minute counter
		if (--sys_timers.counter_1min == 0)
		{
			sys_timers.counter_1min = COUNTER_1MIN_INTERVAL;
			sys_timers.flags |= EXPIRED_1MIN;
			
			// Process auto power off counter
			if (sys_timers.poff_counter != MAX_POWEROFF_TIMEOUT - 1)
				sys_timers.poff_counter++;
			if (sys_timers.poff_counter == p.power_off_timeout - 1)
				sys_timers.flags |= AUTOPOFF_SOON;
			if (sys_timers.poff_counter == p.power_off_timeout)
				sys_timers.flags |= AUTOPOFF_EXPIRED;			
		}
	}	
}


void resetAutoPowerOffCounter(void)
{
	sys_timers.poff_counter = 0;
}


// ----------------------- //
// ----------------------- //




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
	if ( (p.sound_enable) || (enableOverride) )
	{
		beep_cnt = time_ms;
		SetBeepOutput(1);		
	}
	enableOverride = 0;
}

void OverrideSoundDisable(void)
{
	enableOverride = 1;
}

// Stop beeper
void StopBeep()
{
	beep_cnt = 0;
	SetBeepOutput(0);
}





// Period is 1ms @ 16MHz
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
	
	// Start ADC conversion 
	ADCSRA |= (1<<ADSC);
	
}

//==================================================//
//				Sound driver						//
//==================================================//

//void Sound_Play();
//void Sound_Stop();






























