/*
 * power_control.h
 *
 * Created: 27.03.2013 22:32:27
 *  Author: Avega
 */ 


#ifndef POWER_CONTROL_H_
#define POWER_CONTROL_H_

/*		AC line processing time diagram

               AC line zero                                                                    AC line zero
HALF-PERIOD 2   |                              HALF-PERIOD 1                                    |                   HALF-PERIOD 2
                V                                                                               V 
				
                |------------------------------SYNC_IGNORE_TIME-------------------------|---SYNC_LOST_TIMEOUT---|
                |----------QUATER_PERIOD_TIME-----------|                               |                       |
                |---TRIAC_IMPULSE_TIME--|               |                               |                       |	
HEATER  ________|***********************|_______________|_______________________________|_______|***********************|_______
MOTOR   ________________________________________________|************************************************************************
                |										|                                       |
                0 ms                                    5 ms                                   10 ms
*/

// AC line and control timings
#define TRIAC_IMPULSE_TIME			10		// in units of 64 us
#define QUATER_PERIOD_TIME			78		// in units of 64 us
#define SYNC_IGNORE_TIME			125		// in units of 64 us  (125 * 64 = 8000)
#define SYNC_LOST_TIMEOUT			62		// in units of 64 us  (62 * 64 = 3968)

// Regulation params
#define HEATER_MAX_POWER			500		// Heater power control, [0 : HEATER_MAX_POWER]
											// Should be equal to PID controler maximum


//--------------------------------------------//
// FSM and control definitions

// p_state bits
#define HALF_PERIOD_FLAG			0x80
#define STATE_MASK					0x0F


// rollState bits:
// Flags:
#define ROLL_FWD					0x01
#define ROLL_REV					0x02
#define ROLL_CYCLE					0x04	
#define SKIP_CURRENT_MOTOR_CTRL		0x08
// Event bits (sticky flags):
#define CYCLE_ROLL_DONE				0x10	// Set automatically when cycle rolling is done
#define ROLL_DIR_CHANGED			0x20	// Set automatically when top or bottom point is reached and direction is changed

// stopCycleRolling() argument:
#define RESET_POINTS				0x01	// Resets roll points
#define DO_NOT_RESET_POINTS			0x00



//--------------------------------------------//
// Externs

extern uint8_t rollState;			// For read-only access
//#define rollState TWAR
extern uint8_t activeRollCycle;		// For read-only access


// User functions to control heater intensity
void setHeaterPower(uint16_t value);

// User functions to control motor rotation
void setMotorDirection(uint8_t dir);
uint8_t startCycleRolling(void);
void stopCycleRolling(uint8_t doResetPoints);
void clearRollFlags(uint8_t flags);
uint8_t isTopPointValid(void);
uint8_t isBottomPointValid(void);
uint8_t isACSyncPresent(void);

#endif /* POWER_CONTROL_H_ */