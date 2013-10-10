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
#define PID_UPDATE_INTERVAL			(5 * 4)		// in units of CELSIUS_UDPATE_INTERVAL (200ms)		-> 0.2 to 50 seconds


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

//extern SoftTimer8b_t menuUpdateTimer;		// Declared as volatile in main
extern sys_timers_t sys_timers;

void resetAutoPowerOffCounter(void);
void processSystemTimers(void);


//==================================================//
//				Sound driver						//
//==================================================//

// Sound driver FSM states
#define SOUND_OFF	0
#define SOUND_PLAY	1
#define SOUND_APPLY_TONE	2
#define SOUND_START_NEW 3
#define SOUND_DO_BEEP 4
#define SOUND_GET_NEXT_TONE 5

#define TONE_DURATION_SCALE 10				// Single note duration may be up to 2.55 seconds

#define FREQ(x) (125000 / x)				// Macro for specifying timer period from Hz frequency
#define LAST(y)	(y/TONE_DURATION_SCALE)		// Macro for specifying tone duration from ms time units

//#define USE_BEEP_FUNCTION					// If defined, Sound_Beep(freq, time) function is supported

typedef struct
{
	uint8_t tone_period;		// in 16us gradation	
	uint8_t duration;			// in 10ms gradation
} tone_t;

void Sound_Play(const tone_t* p_melody);
void Sound_Stop(void);
void Sound_OverrideDisable(void);

extern const tone_t m_beep_1000Hz_100ms[];
extern const tone_t m_beep_1000Hz_40ms[];
extern const tone_t m_beep_800Hz_40ms[];
extern const tone_t m_beep_500Hz_40ms[];
extern const tone_t m_beep_err1[];
extern const tone_t m_siren1[];
extern const tone_t m_siren2[];
extern const tone_t m_siren3[];
extern const tone_t m_siren4[];
extern const tone_t m_beep_warn_poff[];

// Every melody table must be terminated with tone pair where duration = 0




//-------------------------------//


	








#endif /* SYSTIMER_H_ */