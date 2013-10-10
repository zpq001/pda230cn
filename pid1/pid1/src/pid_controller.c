/*
 * pid_controller.c
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 
 
#include "compilers.h"
#include "pid_controller.h"
#include "fir_filter.h"
  
  

// Nice new model

//TODO: increase resolution of PID output
//+TODO: check code size with integAcc declared as static local or global
//+TODO: pack debug info into structure
//+TODO: optimize log - use pointers, etc
//+TODO: try using of free IO locations


static uint16_t integ_soft_k;

// Sets maximum integrator value for particular temperature setting point in order to reduce wind-up
// Argument is Celsius degree
// Call this function every time when the set point is changed and once during initialization
void setPIDIntegratorLimit(uint8_t set_temp)
{
	// Integrator maximum is computed as integ_soft_k * (INTEGRATOR_SOFT_RANGE - error), see the processPID()
	// When error = 0, maximum is simply integ_soft_k * INTEGRATOR_SOFT_RANGE
	// integ_soft_k is chosen for desired maximum
	// For example, we want integrator maximum of about 30% at 90C. Then integ_soft_k = 300_000 / INTEGRATOR_SOFT_RANGE = 862
	// The "magic" coefficient in the integ_soft_k expression should be 862 / (90 - 15) = 11.49 => 12
	
	if (set_temp < 50)
		set_temp = 50;
	set_temp -= 15;
	integ_soft_k = (uint16_t)set_temp * 12;
}




// Main regulator function - generates output to control plant
// Parameters: 
//	setPoint - desired process value
//	processValue - actual process value
//	mode - enable/disable controller - 
//	  terms are calculated anyway, but output is set to 0 when disabled
uint8_t processPID(uint16_t setPoint, uint16_t processValue, uint8_t mode)
{
	static uint16_t lastProcessValue;	// static locals are initialized with 0
	static int32_t integAcc;			
	int16_t error, p_term, i_term, d_term, temp;
	int32_t integ_max;
	dbg_PID_t* dbg_p = &dbg_PID_struct;
	
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
		integ_max = (int32_t)(INTEGRATOR_SOFT_RANGE - error) * integ_soft_k;	// <- optimized
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
	PRELOAD("z",dbg_p);			// A trick used to make GCC use indirect addressing with displacement
	
	dbg_p->PID_SetPoint = setPoint;
	dbg_p->PID_ProcessValue = processValue;
	//dbg_p->PID_error = error;
	dbg_p->PID_p_term = p_term;
	dbg_p->PID_i_term = i_term;
	//dbg_p->PID_i_max = (int16_t)(integ_max / INTEGRATOR_SCALE);
	dbg_p->PID_d_term = d_term;
	dbg_p->PID_output = temp;

	
	return (uint8_t)temp;	
}






