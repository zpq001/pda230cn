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

// Heater controls
static uint8_t ctrl_heater = 0;			// Heater duty value
static uint8_t ctrl_heater_sync = 0;	// Same, but synchronized to heater regulation period
static uint8_t heater_cnt = 0;			// Counter used to provide heater PWM

// Motor controls
//static uint8_t ctrl_motor = 0;			// Motor control bits


// p_state bits:
//	[7] <- half-period toggling flag
// [3:0] <- state
static uint8_t p_state = 0x0F;			// default error state - if AC line sync is present, 
										// it will be cleared at first comparator ISR
	
// Globals and externs
uint8_t p_flags = 0;					// Module status

uint8_t rollState = 0;
uint8_t borderState = 0;
uint16_t rollCounter = 0;
uint16_t prevRollCounter = 0;
uint8_t activeRollCycle;






uint16_t rollPoint = 32768;


// User function to control heater intensity
inline void setHeaterControl(uint8_t value)
{
	ctrl_heater = value;
	p_flags &= ~READY_TO_UPDATE_HEATER;
	if (value)
		p_flags |= HEATER_ENABLED;
	else
		p_flags &= ~HEATER_ENABLED;
}



	
	
// User function to control motor rotation
void setMotorDirection(uint8_t dir)
{
	// Disable interrupts from timer0 - the only 
	//	possible modifier of rollState excluding this function
	TIMSK &= ~(1<<TOIE0);
		
	if (!(rollState & ROLL_CYCLE))
	{
		rollState &= ~(ROLL_FWD | ROLL_REV);
		rollState |= dir;
	}
	
	if (dir & ROLL_FWD)
		bottomPoint = rollPoint;
	else if (dir & ROLL_REV)
		topPoint = rollPoint;
		

	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);	
}	



uint8_t startCycleRolling(void)
{
	// Disable interrupts from timer0 - the only 
	//	possible modifier of rollState excluding this function
	TIMSK &= ~(1<<TOIE0);
	

	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
	
	return 0;
}

void stopCycleRolling(void)
{
	// Disable interrupts from timer0 - the only 
	//	possible modifier of rollState excluding this function
	TIMSK &= ~(1<<TOIE0);
	

	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
}


uint8_t canCycleRoll(void)
{
	
	
	return 0;
}


//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//

		

	
// Function to process rolling - sets rotation direction for next period
// Call once per each AC line period
static inline void controllRolling()
{
	
	switch(rollState & (ROLL_FWD | ROLL_REV | ROLL_CYCLE))
	{
		case ROLL_FWD:
			rollPoint++;
			break;
		case ROLL_REV:
			rollPoint--;
			break;
			
		case (ROLL_FWD | ROLL_CYCLE):
			rollPoint++;
			if (rollPoint >=  topPoint)
			{
				if (activeRollCycle >= rollCycleSet)	
				{
					// DONE!
					rollState &= ~ROLL_CYCLE;
				}
				else
				{
					activeRollCycle++;
					// Change dir
					rollState &= ~ROLL_FWD;
					rollState |= ROLL_REV;		
				}
			}
			break;
		
		case (ROLL_REV | ROLL_CYCLE):	
			rollPoint--;
			if (rollPoint <=  bottomPoint)
			{
				if (activeRollCycle >= rollCycleSet)	
				{
					// DONE!
					rollState &= ~ROLL_CYCLE;
				}
				else
				{
					activeRollCycle++;
					// Change dir
					rollState &= ~ROLL_FWD;
					rollState |= ROLL_REV;		
				}
			}
			break;
			
		
	}
	
	
	
	
	
	
	// Set flags - actual rotate direction
	p_flags &= ~(ROTATE_FORWARD | ROTATE_REVERSE);
	if (rollState & ROLL_FWD)
		p_flags |= ROTATE_FORWARD;
	else if (rollState & ROLL_REV)
		p_flags |= ROTATE_REVERSE;

}




ISR(ANA_COMP_vect)
{
	// Once triggered, disable further comparator interrupt
	ACSR &= ~(1<<ACIE);
	// Turn on heater TRIAC
	if (heater_cnt < ctrl_heater_sync)
		PORTD |= (1<<PD_HEATER);
	// Reprogram timer0
	TCNT0 = 256 - TRIAC_IMPULSE_TIME;		// Triac gate impulse time
	// Modify state	
	p_state &= ~STATE_MASK;					// Start new state machine cycle
	p_state ^= HALF_PERIOD_FLAG;			// Toggle flag
	
}



ISR(TIMER0_OVF_vect)
{
	uint8_t temp;
	static rollStatePrev = 0;
	
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
			PORTD &= ~(1<<PD_HEATER | 1<<PD_M1 | 1<<PD_M2);
			heater_cnt = 0;
			// TODO
			break;
		
		default:
			// Sync is not present - TODO
			break;
	}	

	// Full period check
	if ((p_state & (HALF_PERIOD_FLAG | STATE_MASK)) == (HALF_PERIOD_FLAG | 0x01))
	{
		// Full AC line period is done. Update controls.
		
		temp = PORTD;
		temp &= ~(1<<PD_M1 | 1<<PD_M2);
		if ( (rollStatePrev ^ rollState) & (ROLL_FWD | ROLL_REV) )
		{
			// Direction control changed. Skip current period to allow TRIACs fully close
			rollStatePrev = rollState;
			PORTD = temp; 	
		}
		else
		{
			// Apply direction control 
			if (rollState & ROLL_FWD)
				temp |= PD_M1;
			else if (rollState & ROLL_REV)
				temp |= PD_M2;
			PORTD = temp; 
			controllRolling();
		}
			

		// Process heater control counter
		if (heater_cnt == HEATER_REGULATION_PERIODS - 1)
		{
			heater_cnt = 0;
			ctrl_heater_sync = ctrl_heater;
			p_flags |= READY_TO_UPDATE_HEATER;
		}
		else
		{
			heater_cnt++;
		}

			
			
	}
	

	if ((p_state & STATE_MASK)  != 0x0F)
		p_state++;

}	














