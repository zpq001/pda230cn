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

// PID d-term filter core
static filter8bit_core_t dterm_filter_core = {	
	.coeffs = {
		64,
		66,
		64,
		59
	},
	.n = 4,
	.dc_gain = 25
};

static int16_t pid_dterm_buffer[4];			// PID d-term buffer	
//static int8_t dterm_coeffs[] = {64,66,64,59};
//static int8_t dterm_coeffs[] = {60,64,67,69,67,64,60,57};

static uint16_t lastProcessValue;
static int32_t integAcc;


void initPID(uint16_t processValue)
{
	uint8_t i;
//	dterm_filter_core.n = 4;
//	dterm_filter_core.dc_gain = 25;
	//dterm_filter_core.n = 8;
	//dterm_filter_core.dc_gain = 512;
//	dterm_filter_core.coeffs = dterm_coeffs;
	for (i=0;i<4;i++)
	{
		pid_dterm_buffer[i] = 0;
	}
	lastProcessValue = processValue;
	integAcc = 0;
}						
 
uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t error, p_term, i_term, d_term, temp;
	//////
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
	#ifdef INTEGRATOR_RANGE_LIMIT
	if ((error >= -INTEGRATOR_ENABLE_RANGE) &&	(error <= INTEGRATOR_ENABLE_RANGE))
	integAcc += error * Ki;
	else
	integAcc = 0;
	#else
	integAcc += error * Ki;
	#endif

	#ifdef INTEGRATOR_SOFT_LIMIT
	// Get the limit value
	//integ_max = (error > INTEGRATOR_SOFT_RANGE) ? INTEGRATOR_SOFT_MAX : INTEGRATOR_MAX;
	if (error > 100)
		integ_max = 0;
	else if (error < 0)
		integ_max = INTEGRATOR_MAX;
	else
	{
		integ_max = (200 - (int32_t)error) * 500;

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
	if (integAcc > INTEGRATOR_MAX )
	{
		integAcc = INTEGRATOR_MAX;
	}
	else if (integAcc < INTEGRATOR_MIN)
	{
		integAcc = INTEGRATOR_MIN;
	}
	#endif
	
	i_term = (int16_t)(integAcc / INTEGRATOR_SCALE);	// Sould not exceed MAXINT16

	//------ Calculate D term --------//
	//d_term = fir_i16_i8((lastProcessValue - processValue), pid_dterm_buffer, &dterm_filter_core);
	d_term = lastProcessValue - processValue;
	d_term = Kd * d_term;

	lastProcessValue = processValue;
	
	//--------- Summ terms -----------//
	temp = (p_term + i_term + d_term) / SCALING_FACTOR;
	
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
	
	
	return temp;
	
}





