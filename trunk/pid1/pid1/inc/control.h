/*
 * control.h
 *
 * Created: 15.04.2013 21:26:56
 *  Author: Avega
 */ 


#ifndef CONTROL_H_
#define CONTROL_H_


typedef struct  
{
	uint8_t setup_temp_value;
	uint8_t rollCycleSet;
	uint8_t sound_enable;
	uint8_t power_off_timeout;
} gParams_t;

typedef struct  
{
	uint8_t cpoint1;
	uint8_t cpoint2;
	uint16_t cpoint1_adc;
	uint16_t cpoint2_adc;
} cParams_t;

// Menu items
#define MAX_SET_TEMP	250
#define MIN_SET_TEMP	30

#define MAX_ROLL_CYCLES	99
#define MIN_ROLL_CYCLES 1

#define MAX_POWEROFF_TIMEOUT	95
#define MIN_POWEROFF_TIMEOUT	5

#define MAX_CALIB_TEMP		250	
#define MIN_CALIB_TEMP		10


// PID regulator

// PID call interval is defined at module systimer.h

//#define Kp	15
//#define Ki	5 
//#define Kd	80 
//#define SCALING_FACTOR	5


//--------------------------------------------//
 
 #define Kp		40
 #define Ki		1
 #define Kd		10
 #define INC_SCALING_FACTOR	100
 //--------------------------------------------//

#define POFF_MOTOR_TRESHOLD	50				// Below this temperature point motor will stop
											// if auto power off mode is active

// Temperature reaching alert range
#define TEMP_ALERT_RANGE	5				// Signal will be active when temperature reach
											// desired value within +-TEMP_ALERT_RANGE Celsius degrees
											// TODO - deliver this to menu
#define TEMP_ALERT_HYST		5				// Hysteresis value for alert range

#define SAFE_TEMP_INTERVAL	15				// Safe interval for growing temperature with heater disabled alert
											// in units of Celsius degree


// heaterState	bits
#define HEATER_ENABLED				0x01
#define CALIBRATION_ACTIVE			0x02

// Auto power off flags
#define AUTO_POFF_ACTIVE	0x01


extern uint8_t heaterState;
extern uint8_t autoPowerOffState;


// Global variables - main system control
extern gParams_t p;		// Global params which are saved to and restored from EEPROM
extern cParams_t cp;		// Calibration params

//------- Debug --------//
extern uint8_t 		dbg_SetPointCelsius;	// Temperature setting, Celsius degree
extern uint16_t 	dbg_SetPointPID;		// Temperature setting, PID input
extern uint8_t 		dbg_RealTempCelsius;	// Real temperature, Celsius degree
extern uint16_t 	dbg_RealTempPID;		// Real temperature, PID input

extern int16_t dbg_PID_p_term;
extern int16_t dbg_PID_d_term;
extern int16_t dbg_PID_i_term;
extern int16_t dbg_PID_output;



void processRollControl(void);
void heaterInit(void);
void processHeaterControl(void);
void processHeaterAlerts(void);
void restoreGlobalParams(void);
void exitPowerOff(void);
void saveCalibrationToEEPROM(void);
//uint8_t processPID(uint16_t setPoint, uint16_t processValue);


#endif /* CONTROL_H_ */