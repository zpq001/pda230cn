/*
 * systimer.h
 *
 * Created: 24.03.2013 14:58:36
 *  Author: Avega
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

// System timers intervals
#define MENU_UPDATE_INTERVAL 			50		// Reference value for all soft system timers, in units of main systimer ticks (1ms)

#define CELSIUS_UDPATE_INTERVAL			4		// in units of MENU_UPDATE_INTERVAL (50ms)			-> 200ms
#define COUNTER_10SEC_INTERVAL			200		// in units of MENU_UPDATE_INTERVAL (50ms)			-> 10s
#define COUNTER_1MIN_INTERVAL			6		// in units of COUNTER_10SEC_INTERVAL (10s)			-> 1m
#define LOG_INTERVAL					2		// in units of MENU_UPDATE_INTERVAL (50ms)			-> 100ms
#define PID_UPDATE_INTERVAL			(5 * 2)		// in units of CELSIUS_UDPATE_INTERVAL (200ms)		-> 0.2 to 50 seconds


// System timers flags
#define EXPIRED_CELSIUS		0x01
#define EXPIRED_10SEC		0x02
#define EXPIRED_1MIN		0x04
#define AUTOPOFF_SOON		0x08
#define AUTOPOFF_EXPIRED	0x10
#define EXPIRED_LOG			0x20
#define UPDATE_PID			0x40



typedef struct {
	uint8_t celsius_upd_counter;			// about 200ms
	uint8_t counter_10sec;		
	uint8_t counter_1min;		
	uint8_t poff_counter;	
	uint8_t log_counter;	
	uint8_t pid_update_counter;
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

void processSystemTimers(void);


//==================================================//
//				Sound driver						//
//==================================================//
/*
typedef struct
{
	uint8_t tone_period;		// in 16us gradation	
	uint8_t duration;			// in 10ms gradation
} tone_t;


	uint8_t repeats;			// 1 - play once, 2 - play twice and so on
*/







#endif /* SYSTIMER_H_ */