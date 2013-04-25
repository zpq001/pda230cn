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


// Regulation params
#define HEATER_REGULATION_PERIODS	10		// Number of whole periods of AC voltage for one heater regulation cycle
#define TRIAC_IMPULSE_TIME			10		// in units of 64 us
#define QUATER_PERIOD_TIME			78		// in units of 64 us
#define SYNC_IGNORE_TIME			140		// in units of 64 us
#define SYNC_LOST_TIMEOUT			32		// in units of 64 us


//---------------------------------//
// User definitions

// setMotorDirection() param
#define ROTATE_FORWARD			0x01		
#define ROTATE_REVERSE			0x02		

// p_flags	bits
#define READY_TO_UPDATE_HEATER	0x80
#define HEATER_ENABLED			0x40
#define ROTATING_FORWARD		0x01		
#define ROTATING_REVERSE		0x02		



//---------------------------------//
// Internal definitions

// p_state bits
#define HALF_PERIOD_FLAG			0x80
#define STATE_MASK					0x0F

// ctrl_motor bits
#define SKIP_CURRENT_MOTOR_REG		0x40
#define CTRL_FORWARD				0x01
#define CTRL_REVERSE				0x02

#define ROLL_FWD					0x01
#define ROLL_REV					0x02
#define ROLL_CYCLE					0x04



extern uint8_t p_flags;					// Module status
extern uint8_t rollState;
extern uint8_t activeRollCycle;

// User function to control motor rotation
void setMotorDirection(uint8_t dir);
// User function to control heater intensity
void setHeaterControl(uint8_t value);

uint8_t startCycleRolling(void);
void stopCycleRolling(void);




#endif /* POWER_CONTROL_H_ */