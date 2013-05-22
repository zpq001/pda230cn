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
	uint8_t cpoint1;
	uint8_t cpoint2;
	uint16_t cpoint1_adc;
	uint16_t cpoint2_adc;
} gParams_t;


#define MAX_SET_TEMP	210
#define MIN_SET_TEMP	30

#define MAX_ROLL_CYCLES	99
#define MIN_ROLL_CYCLES 1

#define MAX_POWEROFF_TIMEOUT	95
#define MIN_POWEROFF_TIMEOUT	5

#define MAX_CALIB_TEMP		250	
#define MIN_CALIB_TEMP		10


// PID regulator
#define Kp	20		// experiment#3 = 40
#define Ki	30
#define SCALING_FACTOR	600


// Global variables - main system control
extern uint16_t setup_temp_value;			// reference temperature
extern uint8_t rollCycleSet;				// number of rolling cycles
extern uint8_t sound_enable;				// Global sound enable
extern uint8_t power_off_timeout;			// Auto power OFF timeout
extern uint8_t cpoint1;						// Calibration point 1
extern uint8_t cpoint2;						// Calibration point 2
extern uint16_t cpoint1_adc;
extern uint16_t cpoint2_adc;

extern uint8_t setTempDbg;				// For UART log only
extern uint8_t pidOutputUpdate;			// For UART log only

void processRollControl(void);
void processHeaterControl(void);

void restoreGlobalParams(void);
void exitPowerOff(void);
void processAutoPowerOff(void);

uint8_t processPID(uint16_t setPoint, uint16_t processValue);


#endif /* CONTROL_H_ */