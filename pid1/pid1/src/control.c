/*
 * control.c
 *
 * Created: 15.04.2013 21:26:33
 *  Author: Avega
 */ 


#include <util/crc16.h>
#include "compilers.h"

#include "control.h"
#include "buttons.h"
#include "power_control.h"
#include "leds.h"
#include "systimer.h"	
#include "adc.h"
#include "pid_controller.h"
#include "usart.h"
#include "port_defs.h"


// Global variables - main system control
// #ifdef USE_EEPROM_CRC
//   Default values must reside in the FLASH memory - for the case when EEPROM is not programmed at all
//   At the very first start after programming MCU default values from FLASH memory are copied to EEPROM and
//   both calibration and global params get protected by 8-bit CRC. EEPROM error message is displayed.
// #endif
EEMEM gParams_t eeGlobalParams = 
{
	.setup_temp_value 	= 50, 
	.rollCycleSet 		= 10,
	.sound_enable 		= 1,
	.power_off_timeout 	= 30,
};

EEMEM cParams_t eeCalibrationParams = 
{
	.cpoint1 			= 24,		
	.cpoint1_adc 		= 796,	
	.cpoint2 			= 130,
	.cpoint2_adc 		= 1672
};

EEMEM uint8_t ee_gParamsCRC = 0xFF;		// Used only when USE_EEPROM_CRC is defined
EEMEM uint8_t ee_cParamsCRC = 0xFF;		// Always declared to avoid changes in EEPROM data map

#ifdef USE_EEPROM_CRC
const PROGMEM gParams_t pmGlobalDefaults =
{
	.setup_temp_value 	= 50,
	.rollCycleSet 		= 10,
	.sound_enable 		= 1,
	.power_off_timeout 	= 30
};

const PROGMEM cParams_t pmCalibrationDefaults = 
{
	.cpoint1 			= 24,		// Default Celsius for the first point
	.cpoint1_adc 		= 796,		// Default ADC for the first point
	.cpoint2 			= 130,
	.cpoint2_adc 		= 1672
};
#endif

gParams_t p;		// Global params which are saved to and restored from EEPROM
					// The global parameters are saved only when device is disconnected from the AC line 
cParams_t cp;		// Calibration params are saved only after calibration of any of two calibration points



//uint8_t heaterState = 0;				// Global heater flags - declared as IO register
uint8_t autoPowerOffState = 0;			// Global flag, active when auto power off mode is active.
										// Flags are set and cleared in menu module.
				
#ifdef MAIN_LOOP_TIME_PROFILING				
uint8_t max_work_time = 0;				// Debug only: maximum time of main loop (ms)						
#endif

//-------------------------------------------------------//
// Function to control motor rotation
//-------------------------------------------------------//
void processRollControl(void)
{	
	uint8_t beepState = 0;
	
	// Process auto power off control
	if (autoPowerOffState & AUTO_POFF_ACTIVE)
	{
		stopCycleRolling(RESET_POINTS);	
		if ( (adc_status & (SENSOR_ERROR_NO_PRESENT | SENSOR_ERROR_SHORTED)) ||
			 (adc_celsius > (POFF_MOTOR_TRESHOLD + POFF_MOTOR_HYST)) )
		{
			// If there is any sensor error, or
			// if temperature is greater than (threshold + some hysteresis) 
			if (!(rollState & (ROLL_FWD | ROLL_REV)))
			{
				// If motor is stopped
				setMotorDirection(ROLL_FWD);		// Start rotating in order to prevent rollers damage
			}
		}
		else if (adc_celsius <= POFF_MOTOR_TRESHOLD)
		{
			setMotorDirection(0);			// Stop the motor
		}
	}
	else
	{
		//-------------------------------------//
		// Control direction by buttons
		if ((buttons.raw_state & (BD_ROTFWD | BD_ROTREV)) == (BD_ROTFWD | BD_ROTREV))
		{
			// Both Forward and Reverse buttons are pressed - stop
			// Attention - stopping motor when rollers are hot can possibly damage them
			setMotorDirection(0);
		}
		else if (buttons.action_down & BD_ROTFWD)
		{
			setMotorDirection(ROLL_FWD);	
			beepState |= 0x01;			// pressed FWD button
		}		
		else if (buttons.action_down & BD_ROTREV)
		{
			setMotorDirection(ROLL_REV);
			beepState |= 0x02;			// pressed REV button
		}		
		else if (autoPowerOffState & AUTO_POFF_LEAVE)
		{
			// Exiting auto power off mode
			if (!(rollState & (ROLL_FWD | ROLL_REV)))
			{
				// Start rotating if motor is stopped
				setMotorDirection(ROLL_FWD);		
			}
		}
			
		//-------------------------------------//
		// Control cycle rolling
		if (buttons.action_long & BD_CYCLE)
		{
			stopCycleRolling(RESET_POINTS);		// Reset points and disable CYCLE mode (if was enabled)
			beepState |= 0x08;					// reset of points by long pressing of ROLL button
		}
		else if (buttons.action_up_short & BD_CYCLE)
		{
			// Disable interrupts from timer0
			//	to prevent rollState from changes - not very beautiful approach
			// Interrupts from Timer0 will be reenabled in either stopCycleRolling() or startCycleRolling()
			TIMSK = (1<<OCIE2);
			if (rollState & ROLL_CYCLE)
			{
				stopCycleRolling(DO_NOT_RESET_POINTS);
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
		
		// ROLL_DIR_CHANGED is set only when direction is changed automatically,
		// not when changed by calling setMotorDirection() function
		// ROLL_DIR_CHANGED and CYCLE_ROLL_DONE flags are sticky
		if (rollState & ROLL_DIR_CHANGED)
		{
			clearRollFlags(ROLL_DIR_CHANGED);
			beepState |= 0x04;	
		}
		if (rollState & CYCLE_ROLL_DONE)
		{
			clearRollFlags(CYCLE_ROLL_DONE);
			beepState |= 0x80;	
		}		
			
		//-------------------------------------//
		// Process sound events
		if (beepState & 0x80)		// Roll cycle done
		{
			Sound_Play(m_siren4);	
		}		
		else if (beepState & 0x40)	// Roll cycle start fail
		{
			Sound_Play(m_beep_500Hz_40ms);	
		} 
		else if (beepState & 0x08)	// Reset points
		{
			Sound_Play(m_beep_800Hz_40ms);	
		}							// Other
		else if ( beepState & (0x01 | 0x02 | 0x10 | 0x20 | 0x04) )
		{
			Sound_Play(m_beep_1000Hz_40ms);	
		}			
			
	}

	//----- LED indication ------//
	clearExtraLeds(LED_ROTFWD | LED_ROTREV);
	if (rollState & ROLL_FWD)
		setExtraLeds(LED_ROTFWD);
	else if (rollState & ROLL_REV)
		setExtraLeds(LED_ROTREV);
}



//-------------------------------------------------------//
// Function to control heater
//-------------------------------------------------------//
void processHeaterControl(void)
{
	uint16_t setPoint;
	uint16_t processValue;
	uint16_t pid_output;
	
	// Process heater ON/OFF control by button
	if (buttons.action_up_short & BD_HEATCTRL)
	{
		heaterState ^= HEATER_ENABLED;
		// Force update heater power
		sys_timers_flags |= UPDATE_PID;		// Not very good approach if UPDATE_PID flag is used outside this function
	}
	
	// Process PID controller reset
	if (buttons.action_long & BD_HEATCTRL)
	{
		heaterState |= RESET_PID;
		// Force update heater power
		sys_timers_flags |= UPDATE_PID;
	}
	else
	{
		heaterState &= ~RESET_PID;
	}
	
	// Process auto power off control and sensor errors
	if ((autoPowerOffState & AUTO_POFF_ACTIVE) || (adc_status & (SENSOR_ERROR_NO_PRESENT | SENSOR_ERROR_SHORTED)))
	{
		heaterState &= ~HEATER_ENABLED;
	}	

	// Update integrator limits if set point is changed
	if (heaterState & SETPOINT_CHANGED)
	{
		setPIDIntegratorLimit(p.setup_temp_value);
		// Force update heater power
		sys_timers_flags |= UPDATE_PID;
	}

	
	// Check if heater control should be updated
	// PID call interval is a multiple of Celsius update interval. 
	if (sys_timers_flags & UPDATE_PID)
	{
		// PID input: 1 count ~ 0.125 Celsius degree (see adc.c)
		setPoint = conv_Celsius_to_ADC(p.setup_temp_value);
		processValue = adc_filtered;
		
		// Process PID
		// If heater is disabled, output will be 0
		pid_output = processPID(setPoint, processValue, heaterState);		
		
		// If unregulated mode is selected, override PID output 
		// This mode must be used with care for calibration only
		if ((heaterState & HEATER_ENABLED) && (p.setup_temp_value >= MAX_SET_TEMP))
			pid_output = HEATER_MAX_POWER;		
			
		// Apply new heater power value	
		setHeaterPower(pid_output);			
	}	
		
	
	//----- LED indication ------//
	if (heaterState & HEATER_ENABLED)
		setExtraLeds(LED_HEATER);
	else
		clearExtraLeds(LED_HEATER);
	
}


//-------------------------------------------------------//
// Function to monitor heater events
//-------------------------------------------------------//
void processHeaterEvents(void)
{
	static uint8_t setPoint_prev = MIN_SET_TEMP + 1;	// Init with value that can never be set
	
	// Generate temperature changed event
	if (setPoint_prev != p.setup_temp_value)
	{
		heaterState |= SETPOINT_CHANGED;
		setPoint_prev = p.setup_temp_value;
	}
	else
	{
		heaterState &= ~SETPOINT_CHANGED;
	}
}



//-------------------------------------------------------//
// Function to process all heater alerts:
//	- sensor errors
//	- getting close to desired temperature
//	- continuous heating when disabled
//-------------------------------------------------------//
void processHeaterAlerts(void)
{
	static uint8_t tempAlertRange = TEMP_ALERT_RANGE;
	static int16_t refCapturedTemp = INT16_MAX;
	int16_t currentTemperature = adc_celsius;
	
	// ADC sensor errors alert
	if (adc_status & (SENSOR_ERROR_NO_PRESENT | SENSOR_ERROR_SHORTED))
	{
		if (sys_timers_flags & EXPIRED_10SEC)
		{
			// Enable beeper output regardless of menu setting
			Sound_OverrideDisable();
			Sound_Play(m_siren3);
		}
		
		// No more alerts should be processed
		return;
	}
	
	
	// Indicate reaching of desired temperature
	if ( (currentTemperature > p.setup_temp_value - tempAlertRange) && (currentTemperature < p.setup_temp_value + tempAlertRange) )
	{
		if ((tempAlertRange == TEMP_ALERT_RANGE) && (heaterState & HEATER_ENABLED))
		{
			Sound_Play(m_siren1);
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
	else if (sys_timers_flags & EXPIRED_10SEC)
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
				Sound_OverrideDisable();
				Sound_Play(m_siren2);
			}
		}
	}
}


//-------------------------------------------------------//
// Function to calculate 8-bit data CRC
//	inputs:
//		void *p 			<- pointer to data
//		uint8_t byte_count	<- number of bytes to use
//	returns:
//		8-bit CRC computed using _crc_ibutton_update() AVR_GCC function
//-------------------------------------------------------//
static uint8_t getDataCRC(void *p,uint8_t byte_count)
{
	uint8_t crc_byte = 0;
	while(byte_count--)
	{
		// Using ibutton CRC function for reason of 8-bit output CRC
		crc_byte = _crc_ibutton_update (crc_byte, *(uint8_t*)p++);	
	}
	return crc_byte;
}


//-------------------------------------------------------//
// Restores global parameters - control and calibration
//	output:
//	- if USE_EEPROM_CRC is not defined, always return 0
//	- if USE_EEPROM_CRC defined:
//		0 if EEPROM parameters are correct
//		1 if control parameters were corrupted and restored from defaults stored in FLASH
//		2 if calibration parameters were corrupted and restored
//		3 if both were corrupted and restored.
//-------------------------------------------------------//
uint8_t restoreGlobalParams(void)
{	
	uint8_t defaults_used = 0;
	
	// Restore global parameters - temperature setting, sound enable, etc.
	eeprom_read_block(&p,&eeGlobalParams,sizeof(gParams_t));
	// Restore ADC calibration parameters
	eeprom_read_block(&cp,&eeCalibrationParams,sizeof(cParams_t));
	
	#ifdef USE_EEPROM_CRC
	uint8_t crc_byte;
	uint8_t temp8u;
	
	//------- Check global params -------//
	crc_byte = getDataCRC(&p,sizeof(gParams_t));
	temp8u = eeprom_read_byte(&ee_gParamsCRC);
	// Restore global defaults if corrupted
	if (temp8u != crc_byte)
	{
		//PGM_read_block(&p,&pmGlobalDefaults,sizeof(gParams_t));
		memcpy_P(&p,&pmGlobalDefaults,sizeof(gParams_t));
		// Save restored default values with correct CRC
		saveGlobalParamsToEEPROM();
		defaults_used |= 0x01;
	}
	
	//----- Check calibration params -----//
	crc_byte = getDataCRC(&cp,sizeof(cParams_t));
	temp8u = eeprom_read_byte(&ee_cParamsCRC);
	// Restore calibration defaults if corrupted
	if (temp8u != crc_byte)
	{
		//PGM_read_block(&cp,&pmCalibrationDefaults,sizeof(cParams_t));
		memcpy_P(&cp,&pmCalibrationDefaults,sizeof(cParams_t));
		// Save restored default values with correct CRC
		saveCalibrationToEEPROM();
		defaults_used |= 0x02;	
	}
	#endif
	
	return defaults_used;
}


//-------------------------------------------------------//
// Saves calibration to EEPROM
//  if USE_EEPROM_CRC is defined, CRC is calculated and stored as well
//-------------------------------------------------------//
void saveCalibrationToEEPROM(void)
{
	// Calibration parameters normally are only saved after calibrating 
	eeprom_update_block(&cp,&eeCalibrationParams,sizeof(cParams_t));	
	#ifdef USE_EEPROM_CRC
	uint8_t new_crc_byte = getDataCRC(&cp,sizeof(cParams_t));
	eeprom_update_byte(&ee_cParamsCRC,new_crc_byte);
	#endif
}

//-------------------------------------------------------//
// Saves global control parameters to EEPROM
//  if USE_EEPROM_CRC is defined, CRC is calculated and stored as well
//-------------------------------------------------------//
void saveGlobalParamsToEEPROM(void)
{
	// Save global parameters to EEPROM
	// eeprom_update_block() updates only bytes that were changed
	eeprom_update_block(&p,&eeGlobalParams,sizeof(gParams_t));
	#ifdef USE_EEPROM_CRC
	uint8_t new_crc_byte = getDataCRC(&p,sizeof(gParams_t));
	eeprom_update_byte(&ee_gParamsCRC,new_crc_byte);
	#endif
}


//-------------------------------------------------------//
// This function is program exit when device has been disconnected from AC line
//  When this function is called, all power-consuming devices like LED display
//	are switched off, and global control parameters are saved to EEPROM
//-------------------------------------------------------//
void exitPowerOff(void)
{
	// Disable all interrupts
	cli();
	
	// Put all ports into HI-Z
	DDRB = 0x00;
	PORTB = 0x00;
	DDRC = 0x00;
	PORTC = 0x00;
	

	PORTD = 0x00;			// Prevent pull-ups
	DDRD = (1<<PD_TXD);
	USART_sendstr("\r\nAC sync lost\r\n");
	
	saveGlobalParamsToEEPROM();
	
	#ifdef MAIN_LOOP_TIME_PROFILING				
	USART_sendstr("Max. main loop time:");
	logU16p(max_work_time);
	USART_sendstr(" ms\r\n");
	#endif
	
	USART_sendstr("Turn OFF");
	
	
	// DIE!
	while(1);
}



