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


EEMEM gParams_t eeGlobalParams = 
{
	.setup_temp_value 	= 50,
	.rollCycleSet 		= 10,
	.sound_enable 		= 1,
	.power_off_timeout 	= 30,
};

EEMEM cParams_t eeCalibrationParams = 
{
	.cpoint1 			= 25,
	//.cpoint1_adc 		= 164,
	.cpoint1_adc 		= 765,
	.cpoint2 			= 145,
	//.cpoint2_adc 		= 433
	.cpoint2_adc 		= 2100
};


gParams_t p;		// Global params which are saved to and restored from EEPROM
					// must be restored at system start
					
cParams_t cp;		// Calibration params



uint8_t autoPowerOffState = 0;


uint16_t pid_dterm_buffer[4];	// PID d-term input buffer
RingBufU16_t ringBufDterm = {
	.length = 4,
	.data = pid_dterm_buffer,
	.stat = RINIT
};


//------- Debug --------//
uint8_t dbg_SetTempCelsius;		// Temperature setting, Celsius degree
uint16_t dbg_SetTempPID;		// Temperature setting, PID input
uint8_t dbg_RealTempCelsius;	// Real temperature, Celsius degree
uint16_t dbg_RealTempPID;		// Real temperature, PID input
//uint16_t dbg_RealTempPIDfiltered;		// Real temperature, PID input, filtered

int16_t dbg_PID_p_term;
int16_t dbg_PID_d_term;
int16_t dbg_PID_i_term;
int16_t dbg_PID_output;


// Function to control motor rotation
void processRollControl(void)
{	
	uint8_t beepState = 0;
//	static uint8_t beepMask = 0x00;
//	uint8_t nextBeepMask = 0xFF;
	static uint8_t force_rotate = 0;
	
	// Process auto power off control
	if (autoPowerOffState & AUTO_POFF_ACTIVE)
	{
		stopCycleRolling(1);	
		if (adc_celsius <= POFF_MOTOR_TRESHOLD)
		{
			force_rotate = ROLL_FWD;		// Default direction
			setMotorDirection(0);			// Stop the motor
		}
	}
	else
	{
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
			stopCycleRolling(1);		// Reset points and disable CYCLE mode (if was enabled)
			beepState |= 0x08;			// reset of points by long pressing of ROLL button
		}
		else if (force_rotate)
		{
			// Auto power off mode was exited by pressing some other button, not direction buttons
			// Start roll, but do not beep in this case
			setMotorDirection(force_rotate);
		}
		force_rotate = 0;		// First normal pass will clear 
			
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
			
		//-----------//
			
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
			
	}

	// Indicate direction by LEDs
	clearExtraLeds(LED_ROTFWD | LED_ROTREV);
	if (rollState & ROLL_FWD)
		setExtraLeds(LED_ROTFWD);
	else if (rollState & ROLL_REV)
		setExtraLeds(LED_ROTREV);
}




void samplePIDProcessValue(void)
{
	PIDsampledADC = getNormalizedRingU16(&ringBufADC);
	//PIDsampledADC = ringBufADC.summ >> 2;
}

void heaterInit(void)
{
	samplePIDProcessValue();
	processPID(0,PIDsampledADC);
}


void processHeaterControl(void)
{
	static uint16_t set_value_adc;		// static for debug
	static uint16_t pid_output;			// static for debug
	
	
	// Process heater ON/OFF control by button
	if (button_state & BD_HEATCTRL)
	{
		heaterState ^= HEATER_ENABLED;
		// Make heater controller set update_flag on next call
		forceHeaterControlUpdate();
	}
	
	// Process auto power off control
	if (autoPowerOffState & AUTO_POFF_ACTIVE)
	{
		heaterState &= ~HEATER_ENABLED;
	}		
	
	
	// Check if heater control should be updated
	// PID call interval is a multiple of AC line periods, computed as HEATER_REGULATION_PERIODS * 20ms * HEATER_PID_CALL_INTERVAL
	if (heaterState & READY_TO_UPDATE_HEATER)
	{
		// Convert temperature setup to equal ADC value
		set_value_adc = conv_Celsius_to_ADC(p.setup_temp_value);					

		// Process PID
		//pid_output = processPID(set_value_adc,PIDsampledADC);
		pid_output = processPID(set_value_adc * 5, adc_filtered);		// oversampled PID control
					
		// Heater control is updated only when flag is set, even if heater must be powered OFF
		if (heaterState & HEATER_ENABLED)
			setHeaterControl(pid_output);	// Flag is cleared automatically
		else
			setHeaterControl(0);
			
		//------- Debug --------//		
		dbg_RealTempPID = adc_normalized;
	}	
		
	
	//------- Debug --------//
	if (heaterState & HEATER_ENABLED)
	{
		setExtraLeds(LED_HEATER);
		dbg_SetTempCelsius = p.setup_temp_value;
		dbg_SetTempPID = set_value_adc;
	}
	else
	{
		dbg_SetTempCelsius = 0;
		dbg_SetTempPID = 0;
		clearExtraLeds(LED_HEATER);
	}
	
	
}









uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t ek;					// current error
	static int16_t yk = 0;		// current PID output
	static int16_t xk_1 = 0;	// processValue at step -1
	static int16_t xk_2 = 0;	// processValue at step -2
	
	int16_t p_term;
	int16_t i_term;
	int16_t d_term;
	
	uint16_t pid_output;
	
	// Calculate error
	ek = setPoint - processValue;
	
	// Calculate PID
	p_term = Kp * (xk_1 - processValue);
	i_term = Ki * ek;
	d_term = Kd * (processValue - 2 * xk_1 + xk_2);
	
	yk += p_term + i_term + d_term;
	xk_2 = xk_1;
	xk_1 = processValue;
	
	// Limit Yk
	if (yk > 1000)
	yk = 1000;
	else if (yk < 0)
	yk = 0;
	
	//------- Debug --------//
	dbg_PID_p_term = p_term;
	dbg_PID_d_term = d_term;
	dbg_PID_i_term = i_term;
	
	pid_output = yk / INC_SCALING_FACTOR;
	
	dbg_PID_output = pid_output;		// full-scale output
	return pid_output;
}


/*

uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t error, p_term, i_term, d_term, temp;
	static uint16_t lastProcessValue;
	static int16_t integAcc = 0;
	
	error = setPoint - processValue;
	
	
	//------ Calculate P term --------//
<<<<<<< .mine
	if (error > 20 )
=======
	if (error > 150)
>>>>>>> .r46
	{
<<<<<<< .mine
		p_term = 1000;
=======
		p_term = 2000;
>>>>>>> .r46
	}
<<<<<<< .mine
	else if (error < -20 )
=======
	else if (error < -150)
>>>>>>> .r46
	{
<<<<<<< .mine
		p_term = -1000 ;
=======
		p_term = -2000;
>>>>>>> .r46
	}
	else
	{
		p_term = error * Kp;
	}
	
	//------ Calculate I term --------//
	integAcc += error;
	if (error <= 0)
	{
		integAcc = 0;
	}
<<<<<<< .mine
	else if (integAcc > 25 )
=======
	else if (integAcc > 200)
>>>>>>> .r46
	{
<<<<<<< .mine
		integAcc = 25;
=======
		integAcc = 200;
>>>>>>> .r46
	}
	else if (integAcc < 0)
	{
		integAcc = 0;
	}
	i_term = integAcc * Ki;

	
	//------ Calculate D term --------//	
	lastProcessValue = ringBufDterm.summ;
	addToRingU16(&ringBufDterm, processValue);
	processValue = ringBufDterm.summ;
	d_term = Kd * ((int16_t)(lastProcessValue - processValue));
	//lastProcessValue = processValue;
	
	//--------- Summ terms -----------//
	temp = (p_term + i_term + d_term) / SCALING_FACTOR;
	
	if (temp > 50)
	{
		temp = 50;	
	}		
	else if (temp < 0)
	{
		temp = 0;
	}
	
	
	//------- Debug --------//
	dbg_PID_p_term = p_term;
	dbg_PID_d_term = d_term;
	dbg_PID_i_term = i_term;
	dbg_PID_output = temp;
	
	
	return temp;
	
}

*/





// Function to process all heater alerts:
//	- getting near to desired temperature
//	- continuous heating when disabled
void processHeaterAlerts(void)
{
	static uint8_t tempAlertRange = TEMP_ALERT_RANGE;
	static uint16_t refCapturedTemp = 0xFFFF;
	uint16_t currentTemperature = adc_celsius;
	
	
	// Indicate reaching of desired temperature
	if ( (currentTemperature > p.setup_temp_value - tempAlertRange) && (currentTemperature < p.setup_temp_value + tempAlertRange) )
	{
		if ((tempAlertRange == TEMP_ALERT_RANGE) && (heaterState & HEATER_ENABLED))
		{
			SetBeeperFreq(1000);
			StartBeep(400);
		}
		// Add some hysteresis
		tempAlertRange = TEMP_ALERT_RANGE + TEMP_ALERT_HYST;
	}			
	else
	{
		tempAlertRange = TEMP_ALERT_RANGE;
	}

	
	// Growing temperature with heater disabled alert 
	// This alert is done regardless of global sound enable
	// A false triggering may occur if ambient temperature grows.
	// To reset the warning in this case just turn on heater for at least one systimer tick (50ms)
	// If heater is enabled, it is implied that user controls heating process
	if (heaterState & (HEATER_ENABLED | CALIBRATION_ACTIVE))
	{
		// Heater enabled, just save current temperature as reference
		// Same if calibration in progress, even if heater is disabled
		refCapturedTemp = currentTemperature;
	}
	else if (sys_timers.flags & EXPIRED_10SEC)
	{
		// Heater disabled. If temperature is falling,
		if (currentTemperature < refCapturedTemp)
		{
			// save current temperature as reference
			refCapturedTemp = currentTemperature;
		}
		else
		{
			// Heater is disabled. If current temperature is higher than reference + some safe interval,
			// there might be a hardware failure - short circuit, etc
			// BEEP like a devil  }:-(
			if (currentTemperature >= (refCapturedTemp + SAFE_TEMP_INTERVAL))
			{
				// Enable beeper output regardless of menu setting
				OverrideSoundDisable();
				SetBeeperFreq(1500);
				StartBeep(5000);	
			}
		}
	}
	

}


void restoreGlobalParams(void)
{
	eeprom_read_block(&p,&eeGlobalParams,sizeof(eeGlobalParams));
	eeprom_read_block(&cp,&eeCalibrationParams,sizeof(eeCalibrationParams));
	 
//	 cpoint1 		= 25;		// TODO: check and remove
//	 cpoint1_adc 	= 164;
//	 cpoint2 		= 145;
//	 cpoint2_adc 	= 433;
	 
}


void saveCalibrationToEEPROM(void)
{
	eeprom_update_block(&cp,&eeCalibrationParams,sizeof(eeCalibrationParams));	
}



void exitPowerOff(void)
{

	// Put all ports into HI-Z
	DDRB = 0x00;
	PORTB = 0x00;
	DDRC = 0x00;
	PORTC = 0x00;
	DDRD = 0x00;
	PORTD = 0x00;
	
	// Disable all interrupts
	cli();
	
	// Save parameters to EEPROM
	eeprom_update_block(&p,&eeGlobalParams,sizeof(eeGlobalParams));	

	// DIE!
	while(1);
}



