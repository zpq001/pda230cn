/*
 * pid_controller.c
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 
 
#include "compilers.h"
#include "pid_controller.h"
#include "fir_filter.h"
  
  

int16_t dbg_PID_p_term;
int16_t dbg_PID_d_term;
int16_t dbg_PID_i_term;
int16_t dbg_PID_output;


// Nice new model

//TODO: increase resolution of PID output
//TODO: check code size with integAcc declared as static local or global


static int32_t integ_soft_k;

// Sets maximum integrator value for particular temperature setting point in order to reduce wind-up
// Argument is Celsius degree
// Call this function every time when the set point is changed and once during initialization
void setPIDIntegratorLimit(uint8_t set_temp)
{
	uint8_t lim_percent;
	if (set_temp < 60)
		lim_percent = 20;
	else
		lim_percent = set_temp / 3;	// 20% for 60C, 30% for 90C, about 53% for 160C

	integ_soft_k = ((int32_t)lim_percent * 10000) / INTEGRATOR_SOFT_RANGE;
}





// Main regulator function - generates output to control plant
// Parameters: 
//	setPoint - desired process value
//	processValue - actual process value
//	mode - enable/disable controller - 
//	  terms are calculated anyway, but output is set to 0 when disabled
uint8_t processPID(uint16_t setPoint, uint16_t processValue, uint8_t mode)
{
	static uint16_t lastProcessValue;
	static int32_t integAcc;
	int16_t error, p_term, i_term, d_term, temp;
	int32_t integ_max;

	// Get the error
	error = setPoint - processValue;
	
	//------ Calculate P term --------//
	if (error > (PROP_MAX / Kp))			// Compare before multiplication to avoid overflow
	{
		p_term = PROP_MAX;	
	}
	else if (error < (PROP_MIN / Kp))
	{
		p_term = PROP_MIN;	
	}
	else
	{
		p_term = error * Kp;
	}
	
	//------ Calculate I term --------//
	if (!(mode & PID_RESET_INTEGRATOR))
		integAcc += error * Ki;
	else
		integAcc = 0;		// May be useful for debug

	#ifdef INTEGRATOR_SOFT_LIMIT
	// Soft limit is a monotone linear function f(error), f(error) = 0 when error = INTEGRATOR_SOFT_RANGE
	// growing up to f(error) = INTEGRATOR_SOFT_MAX when error = 0
	if (error > INTEGRATOR_SOFT_RANGE)
		integ_max = 0;
	else if (error < 0)
		integ_max = INTEGRATOR_MAX;
	else
	{
		//integ_max = (INTEGRATOR_SOFT_RANGE - (int32_t)error) * INTEGRATOR_SOFT_K;
		integ_max = (INTEGRATOR_SOFT_RANGE - (int32_t)error) * integ_soft_k;
	}

	if (integAcc > integ_max )
	{
		integAcc = integ_max;
	}
	else if (integAcc < INTEGRATOR_MIN)
	{
		integAcc = INTEGRATOR_MIN;
	}
	#else
	// Simple limit
	if (integAcc > INTEGRATOR_MAX )
	{
		integAcc = INTEGRATOR_MAX;
	}
	else if (integAcc < INTEGRATOR_MIN)
	{
		integAcc = INTEGRATOR_MIN;
	}
	#endif
	
	i_term = (int16_t)(integAcc / INTEGRATOR_SCALE);	// Should not exceed MAXINT16

	//------ Calculate D term --------//
	d_term = lastProcessValue - processValue;	
	if (d_term > DIFF_MAX / Kd)
	{
		d_term = DIFF_MAX;
	}
	else if (d_term < DIFF_MIN / Kd)
	{
		d_term = DIFF_MIN;
	}
	else
	{
		d_term = Kd * d_term;
	}
	lastProcessValue = processValue;
	
	//--------- Summ terms -----------//
	if (mode & PID_ENABLED)
		temp = (int16_t)( ((int32_t)p_term + (int32_t)i_term + (int32_t)d_term) / SCALING_FACTOR );
	else
		temp = 0;
	
	if (temp > PID_OUTPUT_MAX)
	{
		temp = PID_OUTPUT_MAX;
	}
	else if (temp < PID_OUTPUT_MIN)
	{
		temp = PID_OUTPUT_MIN;
	}
	
	//------- Debug --------//
	dbg_PID_p_term = p_term;
	dbg_PID_d_term = d_term;
	dbg_PID_i_term = i_term;
	dbg_PID_output = temp;
	
	
	return (uint8_t)temp;	
}






