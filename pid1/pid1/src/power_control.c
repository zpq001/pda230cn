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

// Heater controls
static uint8_t ctrl_heater = 0;			// Heater duty value
static uint8_t ctrl_heater_sync = 0;	// Same, but synchronized to heater regulation period
static uint8_t heater_cnt = 0;			// Counter used to provide heater PWM

// Motor controls
static uint16_t rollCnt = 32768;		// Motor period counter used for rolling
static uint8_t ctrl_motor = 0;			// Motor control bits

static RollPoint_t	point_high = {0};
static RollPoint_t	point_low = {0};

// p_state bits:
//	[7] <- half-period toggling flag
// [3:0] <- state
static uint8_t p_state = 0x0F;			// default error state - if AC line sync is present, 
										// it will be cleared at first comparator ISR
										



// Globals and externs
uint8_t p_flags = 0;					// Module status

	
	
// User function to control motor rotation
void setMotorDirection(uint8_t dir)
{
	uint8_t ctrl_motor_prev;
	// Disable interrupts from timer0
	TIMSK &= ~(1<<TOIE0);
	// Save previous control state
	ctrl_motor_prev = ctrl_motor;
	// Modify motor control bits
	ctrl_motor &= ~(CTRL_FORWARD | CTRL_REVERSE);	// clear rotation control bits
	if (dir == ROTATE_FORWARD)
	{
		ctrl_motor |= (CTRL_FORWARD);
	}
	else if (dir == ROTATE_REVERSE)
	{
		ctrl_motor |= (CTRL_REVERSE);
	}
	
	// If control changed, provide one period for TRIAC to close
	if ((ctrl_motor_prev ^ ctrl_motor) & (CTRL_FORWARD | CTRL_REVERSE))
		ctrl_motor |= SKIP_CURRENT_MOTOR_REG;
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);	
}	

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
	
	/*
uint8_t startRepeatRolling(void)
{
	if ((point_high.Valid) && (point_low.Valid))
	{
		if (point_high.Pos > point_low.Pos)
		{
			// Points valid	
		
		}
	}
	
	
	
	return 0;
}

void stopRepeatRolling(void)
{
	// TODO
}
*/



void processRollControl(int8_t inc)
{
	if ( (rollState & (ROLL_FWD | ROLL_REV)) == ROLL_FWD )
		fwdCounter += inc;
	else if ( (rollState & 0x03) == ROLL_REV )
		revCounter += inc;


	

				
}
			
			
void setDirection(uint8_t dir)
{
	if (dir == FORWARD)
	{
			
		
	}
	else
	{
		
	}
	
	prevRollCounter = rollCounter;
	rollCounter = 0;
		
}

void startCycleRolling(void)
{
	
	borderState = ( (rollState & (ROLL_FWD | ROLL_REV)) == ROLL_FWD ) ? ROLL_REV : ROLL_FWD;
	rollState |= ROLL_CYCLE;
}

	
// Function to process rolling - sets rotation direction for next period
// Call once per each AC line period
static inline void controlRolling()
{
	rollCounter++;
	// Check overflow!
	
	if (rollState & ROLL_CYCLE)							// if cycle rolling
	{
		if (rollCounter == prevRollCounter)				// if reached a border
		{
			if (rollState & (ROLL_FWD | ROLL_REV) == borderState)	// if reached border == finish border
			{ 
				if (rollCycleCounter >= rollCycleMax)	// if done desired number of cycles
				{
					rollState &= ~ROLL_CYCLE;			// stop cycle rolling
					// Done!
				}
				else
				{
					rollCycleCounter++;	
					ChangeDirection();	
				}
			}
			else
			{
				ChangeDirection();	
			}
		}
	}
	
	
	
	
	
		
/*	
	// Set flags - actual rotate direction
	p_flags &= ~(ROTATE_FORWARD | ROTATE_REVERSE);
	if (cnt_inc > 0)
		p_flags |= ROTATE_FORWARD;
	else if (cnt_inc < 0)
		p_flags |= ROTATE_REVERSE;
*/
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

		// Output power control - inductive load only
		temp = PORTD;
		temp &= ~(1<<PD_M1 | 1<<PD_M2);
		switch (ctrl_motor & (SKIP_CURRENT_MOTOR_REG | CTRL_FORWARD | CTRL_REVERSE))
		{
			case CTRL_FORWARD:		// Rotating forward
				temp |= (1<<PD_M1);
				PORTD = temp;
			//	controlRolling(1);	// inc counter
				controlRolling();	
				break;
			case CTRL_REVERSE:		// Rotating reverse
				temp |= (1<<PD_M2);	
				PORTD = temp;
			//	controlRolling(-1);	// dec counter
				controlRolling();	
				break;
			default:				// Skip current period to allow TRIACs fully close
				PORTD = temp;
				ctrl_motor &= ~SKIP_CURRENT_MOTOR_REG;
			//	controlRolling(0);	// do not update counter - motor is disabled for current period
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

