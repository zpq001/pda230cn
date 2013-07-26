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
	.cpoint1 			= 25,	// Default Celsius for first point
	.cpoint1_adc 		= 164,	// Normalized ADC for first point
	.cpoint2 			= 145,
	.cpoint2_adc 		= 433
};


gParams_t p;		// Global params which are saved to and restored from EEPROM
					// must be restored at system start
cParams_t cp;		// Calibration params



uint8_t heaterState = 0;				// Global heater flags
uint8_t autoPowerOffState = 0;			// Global flag, active when auto power off mode is active.
										// Flag is set and cleared in menu module.


int16_t pid_dterm_buffer[4];			// PID d-term filter buffer

filter8bit_core_t dterm_filter_core = {
	.coeffs = {
		64,
		66,
		64,
		59
	},
	.n = 4,
	.dc_gain = 256
};


//------- Debug --------//
uint8_t 	dbg_SetPointCelsius;	// Temperature setting, Celsius degree
uint16_t 	dbg_SetPointPID;		// Temperature setting, PID input
uint8_t 	dbg_RealTempCelsius;	// Real temperature, Celsius degree
uint16_t 	dbg_RealTempPID;		// Real temperature, PID input

int16_t dbg_PID_p_term;
int16_t dbg_PID_d_term;
int16_t dbg_PID_i_term;
int16_t dbg_PID_output;

static uint8_t processPID(uint16_t setPoint, uint16_t processValue);




// Function to control motor rotation
void processRollControl(void)
{	
	uint8_t beepState = 0;
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
		if ((raw_button_state & (BD_ROTFWD | BD_ROTREV)) == (BD_ROTFWD | BD_ROTREV))
		{
			// Both Forward and Reverse buttons are pressed - stop
			// Attention - stopping motor when rollers are hot can possibly damage them
			setMotorDirection(0);
		}
		else if (button_action_down & BD_ROTFWD)
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

	//----- LED indication ------//
	clearExtraLeds(LED_ROTFWD | LED_ROTREV);
	if (rollState & ROLL_FWD)
		setExtraLeds(LED_ROTFWD);
	else if (rollState & ROLL_REV)
		setExtraLeds(LED_ROTREV);
}



void heaterInit(void)
{
	// FIXME
	//processPID(0,adc_normalized);
	processPID(0,adc_filtered);			// oversampled PID control
}


void processHeaterControl(void)
{
	uint16_t set_value_adc;	
	uint16_t setPoint;
	uint16_t processValue;
	uint16_t pid_output;
	
	// Process heater ON/OFF control by button
	if (button_state & BD_HEATCTRL)
	{
		heaterState ^= HEATER_ENABLED;
		// Force update heater power
		sys_timers.flags |= UPDATE_PID;		// Not very good approach if UPDATE_PID flag is used somewhere else
	}
	
	// Process auto power off control and sensor errors
	if ((autoPowerOffState & AUTO_POFF_ACTIVE) || (adc_status & (SENSOR_ERROR_NO_PRESENT | SENSOR_ERROR_SHORTED)))
	{
		heaterState &= ~HEATER_ENABLED;
	}		
	
	// Check if heater control should be updated
	// PID call interval is a multiple of Celsius update interval. 
	if (sys_timers.flags & UPDATE_PID)
	{
		// Convert temperature setup to equal ADC value
		set_value_adc = conv_Celsius_to_ADC(p.setup_temp_value);					

		setPoint = set_value_adc * 4;		
		setPoint >>= 1;
		processValue = adc_filtered >> 1;	// normal PID control
		
		// Process PID
		pid_output = processPID(setPoint, processValue);		
					
		// If heater is disabled, override output
		if (!(heaterState & HEATER_ENABLED))
			pid_output = 0;
		// If unregulated mode is selected, set full power
		else if (p.setup_temp_value >= MAX_SET_TEMP)
			pid_output = HEATER_MAX_POWER;		// This mode must be used with care for calibration only
			
		// Set new heater power value	
		setHeaterPower(pid_output);	
		
		
		//------- Debug --------//		
		// PID input:
		dbg_SetPointCelsius = (heaterState & HEATER_ENABLED) ? p.setup_temp_value : 0;
		dbg_SetPointPID = (heaterState & HEATER_ENABLED) ? setPoint : 0;
		dbg_RealTempCelsius = adc_filtered;
		dbg_RealTempPID = processValue;
		// PID output:
		// updated in PID controller function
		
	}	
		
	
	//----- LED indication ------//
	if (heaterState & HEATER_ENABLED)
		setExtraLeds(LED_HEATER);
	else
		clearExtraLeds(LED_HEATER);
	
}




uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t error, p_term, i_term, d_term, temp;
	static uint16_t lastProcessValue;
	static int16_t integAcc = 0;
	
	error = setPoint - processValue;
	
	
	//------ Calculate P term --------//
	
	if (error > 100 )
	{
		p_term = 2000;	
	}
	else if (error < -100 )
	{
		p_term = -2000 ;	
	}
	else
	{
		p_term = error * Kp;
	}
	
	//------ Calculate I term --------//

	integAcc += error;
	
	if (integAcc > 2000 )
	{
		integAcc = 2000;
	}
	else if (integAcc < 0)
	{
		integAcc = 0;
	}
	i_term = integAcc * Ki;
	i_term /= 40;
	
	
	//------ Calculate D term --------//
	// 13_1
	d_term = fir_i16_i8((lastProcessValue - processValue)*10, pid_dterm_buffer, &dterm_filter_core);
	lastProcessValue = processValue;
	d_term = Kd * d_term;
	
	//--------- Summ terms -----------//
	temp = (p_term + i_term + d_term) / SCALING_FACTOR;
	
	if (temp > 100)
	{
		temp = 100;
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



