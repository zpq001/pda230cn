/*
 * control.c
 *
 * Created: 15.04.2013 21:26:33
 *  Author: Avega
 */ 


#include <avr/eeprom.h>
#include "compilers.h"

#include "control.h"
#include "buttons.h"
#include "power_control.h"
#include "led_indic.h"
#include "leds.h"
#include "systimer.h"	
#include "adc.h"




// Global variables - main system control


EEMEM gParams_t nvParams = 
{
	.setup_temp_value = 50,
	.rollCycleSet = 10,
	.sound_enable = 1,
	.power_off_timeout = 30,
	.cpoint1 		= 25,
	.cpoint1_adc 	= 860,
	.cpoint2 		= 145,
	.cpoint2_adc 	= 591
};

uint16_t setup_temp_value;		// reference temperature
uint8_t rollCycleSet;			// number of rolling cycles
uint8_t sound_enable;			// Global sound enable
uint8_t power_off_timeout;		// Auto power OFF timeout, minutes
uint8_t cpoint1;				// Calibration point 1, Celsius degree
uint8_t cpoint2;				// Calibration point 2, Celsius degree
uint16_t cpoint1_adc;			// Calibration point 1, ADC value
uint16_t cpoint2_adc;			// Calibration point 2, ADC value


// Function to control motor rotation
void processRollControl(void)
{	
	uint8_t beepState = 0;
	static uint8_t beepMask = 0x00;
	
	// Control direction by buttons
	if (button_action_down & BD_ROTFWD)
	{
		setMotorDirection(ROLL_FWD);	
		beepState |= 0x01;			// pressed FWD button
	}		
	else if (button_action_down & BD_ROTREV)
	{
		setMotorDirection(ROLL_REV);
		beepState |= 0x02;			// pressed REV button
	}		
	else if (button_action_long & BD_CYCLE)
	{
		stopCycleRolling(1);		// Reset points and disabled CYCLE mode (if was enabled)
		beepState |= 0x08;			// reset of points by long pressing of ROLL button
	}
	
	
	if (button_action_up_short & BD_CYCLE)
	{
		if (rollState & ROLL_CYCLE)
		{
			stopCycleRolling(0);
			beepState |= 0x20;		// stopped cycle
		}
		else if (startCycleRolling())
		{
			beepState |= 0x10;		// started cycle
		}
		else
		{
			beepState |= 0x40;		// failed to start cycle
		}			
	}		
	
	if (rollState & ROLL_DIR_CHANGED)
	{
		rollState &= ~ROLL_DIR_CHANGED;
		beepState |= 0x04;	
	}
	
	if (rollState & CYCLE_ROLL_DONE)
	{
		rollState &= ~CYCLE_ROLL_DONE;
		beepState |= 0x80;	
	}		
	
	beepState &= beepMask;
	
	if (beepState & 0x80)		// Roll cycle done
	{
		SetBeeperFreq(1000);
		StartBeep(200);
	}		
	else if (beepState & 0x40)	// Roll cycle start fail
	{
		SetBeeperFreq(500);
		StartBeep(50);
	} 
	else if (beepState & 0x08)	// Reset points
	{
		SetBeeperFreq(800);
		StartBeep(50);
	}							// Other
	else if ( beepState & (0x01 | 0x02 | 0x10 | 0x20 | 0x04) )
	{
		SetBeeperFreq(1000);
		StartBeep(50);	
	}			
	
	// Apply mask to next sound events
	beepMask = 0xFF;
	// Disable beep from DIR_CHANGED on next call if direction buttons have been pressed
	if (beepState & 0x03)	
		beepMask &= ~0x04;	
		
	
		
	// Indicate direction by LEDs
	clearExtraLeds(LED_ROTFWD | LED_ROTREV);
	if (rollState & ROLL_FWD)
		setExtraLeds(LED_ROTFWD);
	else if (rollState & ROLL_REV)
		setExtraLeds(LED_ROTREV);
		
}





void processHeaterControl(void)
{
	static uint8_t heater_ctrl = 0;
	uint16_t set_value_adc;
	
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
		
		if (heaterState & READY_TO_UPDATE_HEATER)
		{
			
			// Convert temperature setup to equal ADC value
			set_value_adc = conv_Celsius_to_ADC(setup_temp_value);
			
			// PID !!!
			setHeaterControl(10);	
			
			heaterState &= ~READY_TO_UPDATE_HEATER;
		}
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






uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	
	int16_t error, p_term;
	
	error = setPoint - processValue;
	
	
	//------ Calculate P term --------//
	if (error > 
	
	
	
	
	
}




void restoreGlobalParams(void)
{
	 gParams_t gParams;
	 eeprom_read_block(&gParams,&nvParams,sizeof(nvParams));
	 setup_temp_value = gParams.setup_temp_value;	// reference temperature
	 rollCycleSet = gParams.rollCycleSet;			// number of rolling cycles
	 sound_enable = gParams.sound_enable;			// Global sound enable
	 power_off_timeout = gParams.power_off_timeout;	// Auto power OFF timeout, minutes
	 cpoint1 = gParams.cpoint1;						// Calibration point 1
	 cpoint2 = gParams.cpoint2;						// Calibration point 2
	 cpoint1_adc = gParams.cpoint1_adc;
	 cpoint2_adc = gParams.cpoint2_adc;
	 
	 cpoint1 		= 25;
	 cpoint1_adc 	= 860;
	 cpoint2 		= 145;
	 cpoint2_adc 	= 591;
	 
}




void exitPowerOff(void)
{
	gParams_t gParams;
	/*
	// Put all ports into HI-Z
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x00;
	
	// Disable all interrupts
	cli();
	*/
	
	// Save parameters to EEPROM
/*	gParams.setup_temp_value = setup_temp_value;
	gParams.rollCycleSet = rollCycleSet;
	gParams.sound_enable = sound_enable;
	gParams.power_off_timeout = power_off_timeout;
	gParams.cpoint1 = cpoint1;
	gParams.cpoint2 = cpoint2;
	gParams.cpoint1_adc = cpoint1_adc;
	gParams.cpoint2_adc = cpoint2_adc;
	eeprom_update_block(&gParams,&nvParams,sizeof(nvParams));	

	while(1);
*/
}


void processAutoPowerOff(void)
{
	//TODO
}


