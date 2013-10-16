/*
 * control.h
 *
 * Created: 15.04.2013 21:26:56
 *  Author: Avega
 */ 


#ifndef CONTROL_H_
#define CONTROL_H_

//--------------------------------------------//
// Typedefs

// Global control parameters structure type
typedef struct  
{
	uint8_t setup_temp_value;
	uint8_t rollCycleSet;
	uint8_t sound_enable;
	uint8_t power_off_timeout;
} gParams_t;

// Global calibration parameters structure type
typedef struct  
{
	uint8_t cpoint1;
	uint8_t cpoint2;
	uint16_t cpoint1_adc;
	uint16_t cpoint2_adc;
} cParams_t;


//--------------------------------------------//
// Limits and parameters for menu regulation

//---------- HEATER -----------//
#define MAX_SET_TEMP	255		// Actual maximum temperature for stabilization will be (MAX_SET_TEMP - TEMP_SET_STEP)
								// When MAX_SET_TEMP is set, heater is constantly enabled
#define MIN_SET_TEMP	30
#define TEMP_SET_STEP	5

//---------- ROLLING ----------//
#define MAX_ROLL_CYCLES	99
#define MIN_ROLL_CYCLES 1
#define ROLL_CYCLES_STEP 1

//--------- POWER OFF ---------//
#define MAX_POWEROFF_TIMEOUT	95	// Actual maximum timeout will be (MAX_POWEROFF_TIMEOUT - POWEROFF_SET_STEP)
									// When MAX_POWEROFF_TIMEOUT is set, auto power off mode is disabled
#define MIN_POWEROFF_TIMEOUT	5
#define POWEROFF_SET_STEP		5

//------- CALIBRATION ---------//
#define MAX_CALIB_TEMP		250	
#define MIN_CALIB_TEMP		10
#define CALIB_TEMP_STEP		1



//--------------------------------------------//
// Other system parameters

// Power off mode settings for rolling
#define POFF_MOTOR_TRESHOLD	50				// Below this temperature point motor will stop
											// if auto power off mode is active
#define POFF_MOTOR_HYST		5

// Temperature reaching alert range
#define TEMP_ALERT_RANGE	5				// Signal will be active when temperature reach
											// desired value within +-TEMP_ALERT_RANGE Celsius degrees
#define TEMP_ALERT_HYST		5				// Hysteresis value for alert range

#define SAFE_TEMP_INTERVAL	20				// Safe interval for growing temperature with heater disabled alert
											// in units of Celsius degree

#define USE_EEPROM_CRC						// CRC will be used for EEPROM parameter protection


//--------------------------------------------//
// Global control and status variables bits
											
// rollState bits are found in power_control.h

// heaterState	bits
#define HEATER_ENABLED			0x01	// must be equal to PID_ENABLED
#define CALIBRATION_ACTIVE		0x02	
#define RESET_PID				0x04	// must be equal to PID_RESET_INTEGRATOR
#define SETPOINT_CHANGED		0x08

// autoPowerOffState bits
#define AUTO_POFF_ACTIVE		0x01	// Set continuously while device stays in power off mode
#define AUTO_POFF_LEAVE			0x02	// Set when device is leaving power off mode


//--------------------------------------------//
// Externs

// Global variables - main system control
extern gParams_t p;						// Global params 
extern cParams_t cp;					// Calibration params

//extern uint8_t heaterState;			
#define heaterState TWBR				// Initialized in init_system_io()

extern uint8_t autoPowerOffState;	


void processRollControl(void);
void processHeaterControl(void);
void processHeaterEvents(void);
void processHeaterAlerts(void);
uint8_t restoreGlobalParams(void);		// Returns zero if EEPROM data CRC is correct
void exitPowerOff(void);
void saveCalibrationToEEPROM(void);
void saveGlobalParamsToEEPROM(void);


#endif /* CONTROL_H_ */