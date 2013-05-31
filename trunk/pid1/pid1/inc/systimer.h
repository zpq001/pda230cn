/*
 * systimer.h
 *
 * Created: 24.03.2013 14:58:36
 *  Author: Avega
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

// System timers intervals
#define MENU_UPDATE_INTERVAL 	50			// in units of main systimer ticks (1ms)
#define SYSTICKS_PER_SECOND		(1000 / MENU_UPDATE_INTERVAL)

#define CELSIUS_UDPATE_INTERVAL			4		// in units of MENU_UPDATE_INTERVAL (50ms)			-> 200ms
#define COUNTER_10SEC_INTERVAL		20//	200		// in units of MENU_UPDATE_INTERVAL (50ms)			-> 10s
#define COUNTER_1MIN_INTERVAL			6		// in units of TEMP_ALERT_PROCESS_INTERVALL (10s)	-> 1m
#define LOG_INTERVAL					2		// in units of MENU_UPDATE_INTERVAL (50ms)			-> 100ms

// System timers flags
#define EXPIRED_CELSIUS		0x01
#define EXPIRED_10SEC		0x02
#define EXPIRED_1MIN		0x04
#define AUTOPOFF_SOON		0x08
#define AUTOPOFF_EXPIRED	0x10
#define EXPIRED_LOG			0x20

// Auto power off flags
#define AUTO_POFF_ACTIVE	0x01
#define AUTO_POFF_ENTER		0x02
#define AUTO_POFF_LEAVE		0x04
#define RESET_COUNTER		0x10



typedef struct {
	uint8_t celsius_upd_counter;			// about 200ms
	uint8_t counter_10sec;		
	uint8_t counter_1min;		
	uint8_t poff_counter;	
	uint8_t log_counter;	
	uint8_t flags;
} sys_timers_t;


extern uint8_t minute_counter;
extern sys_timers_t sys_timers;

void StartBeep(uint16_t time_ms);
void StopBeep();
void OverrideSoundDisable(void);

// Use either one to setup beeper frequency
// Beeper frequency is not precise !!!
void SetBeeperPeriod(uint16_t new_period_us);
void SetBeeperFreq(uint16_t freq_hz);

void resetAutoPowerOffCounter(void);



#endif /* SYSTIMER_H_ */