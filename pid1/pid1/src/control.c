/*
 * control.c
 *
 * Created: 15.04.2013 21:26:33
 *  Author: Avega
 */ 



#include "compilers.h"

#include "control.h"
#include "buttons.h"
#include "power_control.h"
#include "led_indic.h"
#include "leds.h"
#include "systimer.h"	




// Global variables - main system control
uint16_t setup_temp_value = 65;		// reference temperature
uint8_t rollCycleSet = 10;			// number of rolling cycles
uint8_t sound_enable = 0;			// Global sound enable
uint8_t power_off_timeout = 30;		// Auto power OFF timeout, minutes
uint8_t cpoint1 = 25;				// Calibration point 1
uint8_t cpoint2 = 180;				// Calibration point 2



// Function to control motor rotation
void processRollControl(void)
{	
	
	// Control direction by buttons
	if (button_state & BD_ROTFWD)
		setMotorDirection(ROLL_FWD);
	else if (button_state & BD_ROTREV)
		setMotorDirection(ROLL_REV);
		
	// TODO: add reset of points by long pressing of ROLL button
		
	if (button_action_down & 0x80)
	{
		if (rollState & ROLL_CYCLE)
		{
			stopCycleRolling();
			SetBeeperFreq(1000);
			StartBeep(50);
		}
		else if (startCycleRolling())
		{
			SetBeeperFreq(1000);
			StartBeep(100);
		}
		else
		{
			SetBeeperFreq(500);
			StartBeep(50);
		}
	}
		
		
	// Indicate direction by LEDs
	clearExtraLeds(LED_ROTFWD | LED_ROTREV);
	if (rollState & ROLL_FWD)
		setExtraLeds(LED_ROTFWD);
	else if (rollState & ROLL_REV)
		setExtraLeds(LED_ROTREV);
	
		
}


void processHeaterControl(void)
{
	// p_flags & HEATER_ENABLED
	static uint8_t heater_ctrl = 0;
	
	// Process heater ON/OFF control by button
	if (button_state & BD_HEATCTRL)
	{
		heater_ctrl ^= 0x01;
	}
	
	
	if (heater_ctrl)
	{
		// Heater enabled

		//-----------------//
		// Process PID
		//// TODO!!!!
		setHeaterControl(10);
		//-----------------//
		
		setExtraLeds(LED_HEATER);
	}
	else
	{
		// Heater disabled
		setHeaterControl(0);
		clearExtraLeds(LED_HEATER);
	}
}





