/*
 * power_control.c
 *
 *	Module for heater and motor control
 *
 * Created: 27.03.2013 22:32:13
 *  Author: Avega
 */ 

#include "compilers.h"
#include "port_defs.h"
#include "power_control.h"
#include "control.h"
#include "adc.h"

// Heater controls
uint8_t ctrl_heater = 0;				// Heater duty value
static uint8_t ctrl_heater_sync = 0;	// Same, but synchronized to heater regulation period
static uint8_t heater_cnt = 0;			// Counter used to provide heater PWM
uint8_t heaterState = 0;				// Global heater flags
uint8_t heater_reg_cnt = 0;


// Motor controls
uint8_t rollState = 0;					// Roll controller state. Use as read-only
uint8_t activeRollCycle = 0;			// Indicates currently active roll cycle
static uint8_t newDirReq = 0;
static uint16_t rollPoint = 0;
static uint16_t topPoint = 0;
static uint16_t bottomPoint = 0;
static uint8_t dirChangedMask = 0xFF;

// p_state bits:
//	[7] <- half-period toggling flag
// [3:0] <- state
uint8_t p_state = 0x0F;			// default state - if AC line sync is present, 
								// it will be cleared at first comparator ISR call



// User function to control heater intensity
void setHeaterControl(uint8_t value)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
	
	ctrl_heater = value;
	heaterState &= ~READY_TO_UPDATE_HEATER;
	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);	
}


void forceHeaterControlUpdate(void)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
	
	// Flag READY_TO_UPDATE_HEATER will be set on next on next AC line period
	heater_cnt = HEATER_REGULATION_PERIODS - 6;
	heater_reg_cnt = HEATER_PID_CALL_INTERVAL - 1;
	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);	
}
	
	
// User function to control motor rotation
void setMotorDirection(uint8_t dir)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
		
	newDirReq = dir;	// save new direction request
	dirChangedMask = ~ROLL_DIR_CHANGED;
	
	if (dir & ROLL_FWD)
		bottomPoint = rollPoint;
	else if (dir & ROLL_REV)
		topPoint = rollPoint;

	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);	
}	



uint8_t startCycleRolling(void)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
	
	if ( isTopPointValid() && isBottomPointValid() )
	{
		rollState |= ROLL_CYCLE;
		activeRollCycle = 1;
	}
	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
	
	return (rollState & ROLL_CYCLE);
}

void stopCycleRolling(uint8_t doResetPoints)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
	
	rollState &= ~ROLL_CYCLE;
	if (doResetPoints)
	{
		topPoint = bottomPoint = rollPoint;
		activeRollCycle = 0;	
	}		
	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
}



uint8_t isTopPointValid(void)
{
	return (	(int16_t)(topPoint - rollPoint) >= 0 );
}

uint8_t isBottomPointValid(void)
{
	return (	(int16_t)(rollPoint - bottomPoint) >= 0	);
}

//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//

static inline uint8_t reachedTopPoint(void)
{
	return (	(int16_t)(topPoint - rollPoint) <= 0 );
}

static inline uint8_t reachedBottomPoint(void)
{
	return (	(int16_t)(rollPoint - bottomPoint) <= 0 );
}

static inline void updateRollPoint(void)
{	
	if (rollState & ROLL_FWD)
		rollPoint++;
	else if (rollState & ROLL_REV)
		rollPoint--;	
}
		


// Function to process rolling - sets rotation direction for next period
// Call once per each AC line period
static inline void controlRolling()
{
	// Process cycle rolling
	switch(rollState & (ROLL_FWD | ROLL_REV | ROLL_CYCLE))
	{
		case (ROLL_FWD | ROLL_CYCLE):
			if (reachedTopPoint())
			{
				if (activeRollCycle >= rollCycleSet)	
				{
					// DONE!
					rollState &= ~ROLL_CYCLE;
					rollState |= CYCLE_ROLL_DONE;
				}
				else
				{
					activeRollCycle++;
					// Change dir	
					newDirReq = ROLL_REV;				
				}
			}
			break;
		
		case (ROLL_REV | ROLL_CYCLE):	
			if (reachedBottomPoint())
			{
				if (activeRollCycle >= rollCycleSet)	
				{
					// DONE!
					rollState &= ~ROLL_CYCLE;
					rollState |= CYCLE_ROLL_DONE;
				}
				else
				{
					activeRollCycle++;
					// Change dir	
					newDirReq = ROLL_FWD;
				}
			}
			break;
			
		default:
			break;
	}
	
	// Process direction change
	if ((rollState ^ newDirReq) & (ROLL_FWD | ROLL_REV))
	{
		// ROLL_DIR_CHANGED is used for sound beep
		rollState |= (SKIP_CURRENT_MOTOR_CTRL | ROLL_DIR_CHANGED);
	}
	
	rollState &= ~(ROLL_FWD | ROLL_REV);
	rollState |= newDirReq;
	rollState &= dirChangedMask;
	dirChangedMask = 0xFF;
	
	updateRollPoint();
}




ISR(ANA_COMP_vect)
{
	// Once triggered, disable further comparator interrupt
	ACSR &= ~(1<<ACIE);
	// Turn on heater TRIAC
	if (heater_cnt < ctrl_heater_sync)
		PORTD |= (1<<PD_HEATER | 1<<PD_HEAT_INDIC);	// Direct heater indication
	else
		PORTD &= ~(1<<PD_HEAT_INDIC);
	// Reprogram timer0
	TCNT0 = 256 - TRIAC_IMPULSE_TIME;		// Triac gate impulse time
	// Modify state	
	p_state &= ~STATE_MASK;					// Start new state machine cycle
	p_state ^= HALF_PERIOD_FLAG;			// Toggle flag
	
}



ISR(TIMER0_OVF_vect)
{
	uint8_t temp;
	
	switch(p_state & STATE_MASK)
	{
		// TRIAC_IMPULSE_TIME finished
		case 0x00:
			// Turn off heater TRIAC
			PORTD &= ~(1<<PD_HEATER);
			TCNT0 = 256 - (QUATER_PERIOD_TIME - TRIAC_IMPULSE_TIME);
			break;
		// QUATER_PERIOD_TIME finished	
		case 0x01:	
			TCNT0 = 256 - (SYNC_IGNORE_TIME - QUATER_PERIOD_TIME);
			break;	
		// SYNC_IGNORE_TIME finished	
		case 0x02:
			TCNT0 = 256 - SYNC_LOST_TIMEOUT;
			// Clear flag and enable interrupt from analog comparator
			ACSR |= (1<<ACI);
			ACSR |= (1<<ACIE);
			break;
		// SYNC_LOST_TIMEOUT finished
		case 0x03:
			// Error - sync is lost. It means that device has been disconnected from AC line.
			// Disable all power-consuming devices like LEDs and save user settings to EEPROM.
			exitPowerOff();
			break;
		
		default:
			// Sync is not present - do nothing
			break;
	}	

	// Full period check
	if ((p_state & (HALF_PERIOD_FLAG | STATE_MASK)) == (HALF_PERIOD_FLAG | 0x01))
	{
		// Full AC line period is done. Update controls.
		
		temp = PORTD;
		temp &= ~(1<<PD_M1 | 1<<PD_M2);
		if ( rollState & SKIP_CURRENT_MOTOR_CTRL )
		{
			// Direction control changed. Skip current period to allow TRIACs fully close
			rollState &= ~SKIP_CURRENT_MOTOR_CTRL;
			PORTD = temp; 	
		}
		else
		{
			// Apply direction control 
			if (rollState & ROLL_FWD)
				temp |= (1<<PD_M1);
			else if (rollState & ROLL_REV)
				temp |= (1<<PD_M2);
			PORTD = temp; 
			controlRolling();
		}
			

		// Process heater control 
		if (heater_cnt == HEATER_REGULATION_PERIODS - 6)
		{
			 if (heater_reg_cnt == HEATER_PID_CALL_INTERVAL - 1)
			 {
				 heater_reg_cnt = 0;
				 // Set flag for PID control
				 heaterState |= READY_TO_UPDATE_HEATER;
				 // Save temperature measure at current time
				 PIDsampledADC = getNormalizedRingU16(&ringBufADC);
			 }
			 else
			 {
				 heater_reg_cnt++;
			 }
		}			 
		
		
		if (heater_cnt == HEATER_REGULATION_PERIODS - 1)
		{
			heater_cnt = 0;
			// Copy new output value
			ctrl_heater_sync = ctrl_heater;
		}
		else
		{
			heater_cnt++;
		}

			
			
	}
	

	if ((p_state & STATE_MASK)  != 0x0F)
		p_state++;

}	














