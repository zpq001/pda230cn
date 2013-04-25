/*
 * systimer.h
 *
 * Created: 24.03.2013 14:58:36
 *  Author: Avega
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#define MENU_UPDATE_INTERVAL 	50	// in units of systimer ticks



void StartBeep(uint16_t time_ms);
void StopBeep();

// Use either one to setup beeper frequency
// Beeper frequency is not precise !!!
void SetBeeperPeriod(uint16_t new_period_us);
void SetBeeperFreq(uint16_t freq_hz);




#endif /* SYSTIMER_H_ */