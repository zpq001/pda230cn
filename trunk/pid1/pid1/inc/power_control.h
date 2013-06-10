/*
 * power_control.h
 *
 * Created: 27.03.2013 22:32:27
 *  Author: Avega
 */ 


#ifndef POWER_CONTROL_H_
#define POWER_CONTROL_H_

/*
					
				|------------------------------SYNC_IGNORE_TIME-------------------------|---SYNC_LOST_TIMEOUT---|
				|----------QUATER_PERIOD_TIME-----------|								|					   	|
				|---TRIAC_IMPULSE_TIME--|				|								|						|	
HEATER	________|***********************|_______________|_______________________________|_______|***********************|_______
MOTOR	________________________________________________|************************************************************************
				|										|										|
				0 ms									5 ms									10 ms
*/


#define TRIAC_IMPULSE_TIME			10		// in units of 64 us
#define QUATER_PERIOD_TIME			78		// in units of 64 us
#define SYNC_IGNORE_TIME			140		// in units of 64 us
#define SYNC_LOST_TIMEOUT			32		// in units of 64 us


// Regulation params
#define HEATER_MAX_POWER			100		// Heater power control, [0 : HEATER_MAX_POWER]


//---------------------------------//
// Internal definitions

// p_state bits
#define HALF_PERIOD_FLAG			0x80
#define STATE_MASK					0x0F


//---------------------------------//
// User definitions


// rollState bits:
// Flags:
#define ROLL_FWD					0x01
#define ROLL_REV					0x02
#define ROLL_CYCLE					0x04	
#define SKIP_CURRENT_MOTOR_CTRL		0x08
// Event bits:
#define CYCLE_ROLL_DONE				0x10
#define ROLL_DIR_CHANGED			0x20	// Set only automatically during cycle rolling



extern uint8_t p_state;				// For read-only
extern uint8_t rollState;			
extern uint8_t activeRollCycle;


// User functions to control heater intensity
void setHeaterPower(uint16_t value);

// User functions to control motor rotation
void setMotorDirection(uint8_t dir);
uint8_t startCycleRolling(void);
void stopCycleRolling(uint8_t doResetPoints);
uint8_t isTopPointValid(void);
uint8_t isBottomPointValid(void);


#endif /* POWER_CONTROL_H_ */