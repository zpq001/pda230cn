/*
 * systimer.h
 *
 * Created: 24.03.2013 14:58:36
 *  Author: Avega
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#define MENU_UPDATE_INTERVAL 	50	// in units of systimer ticks
#define SYSTICKS_PER_SECOND		(1000 / MENU_UPDATE_INTERVAL)
#define CELSIUS_UDPATE_INTERVAL	4	// in units of MENU_UPDATE_INTERVAL

// Auto power off flags
#define AUTO_POFF_ACTIVE	0x01
#define AUTO_POFF_ENTER		0x02
#define AUTO_POFF_LEAVE		0x04
#define RESET_COUNTER		0x10

extern uint8_t autoPowerOffState;
extern uint8_t minute_counter;

void StartBeep(uint16_t time_ms);
void StopBeep();

// Use either one to setup beeper frequency
// Beeper frequency is not precise !!!
void SetBeeperPeriod(uint16_t new_period_us);
void SetBeeperFreq(uint16_t freq_hz);

void processAutoPowerOff(void);
void resetAutoPowerOffCounter(void);



#endif /* SYSTIMER_H_ */