/*
 * pid_controller.c
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 

#include "compilers.h"
#include "math.h"
#include "pid_controller.h"
#include "fir_filter.h"

#include "stdlib.h"
#include <memory.h>

/////////// MEDIAN ///////////
#define MEDIAN_N 5
int16_t mbuf[MEDIAN_N];
int16_t get_median_i16(int16_t *buf, uint8_t n)
{
	int16_t temp;
	int16_t *tempbuf = malloc(sizeof(int16_t)*n);
	int i,j;
	memcpy(tempbuf,buf,sizeof(int16_t)*n);
	// Sort buffer
	for (i = 0; i < n-1; i++)
	{
		for (j = 0; j < n-1; j++)
		{
			if (tempbuf[j] < tempbuf[j+1])
			{
				temp = tempbuf[j];
				tempbuf[j] = tempbuf[j+1];
				tempbuf[j+1] = temp;
			}
		}
	}

	temp = tempbuf[n/2];
	free(tempbuf);
	return temp;
}

void push_i16(int16_t *buf, uint8_t n, int16_t new_val)
{
	int i;
	for (i = n-1; i > 0; i--)
	{
		buf[i] = buf[i-1];
	}
	buf[0] = new_val;
}



#ifdef PID_INCREMENTAL 

 
int16_t dbg_PID_p_term;
int32_t dbg_PID_d_term;
int32_t dbg_PID_i_term;
int16_t dbg_PID_output;


#define Kp 8
#define P_scale	2000

#define Ki 100
#define I_scale 1

#define Kd 10
#define D_scale 2000

#define OUTPUT_SCALE 20000
// Output limits
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 100

int32_t U;
int32_t E_prev;
int32_t E_prev2;


void initPID(uint16_t processValue)
{
	U = 0;
	E_prev = 0;
	E_prev2 = 0;
	lastProcessValue = processValue;
}						
 
uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t error, p_term, i_term, d_term, temp;
	
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
		integAcc += (int32_t)error * Ki;	
	else
		integAcc = 0;	
	#else
	integAcc += (int32_t)error * Ki;	
	#endif
		
	if (integAcc > INTEGRATOR_MAX )
	{
		integAcc = INTEGRATOR_MAX;
	}
	else if (integAcc < INTEGRATOR_MIN)
	{
		integAcc = INTEGRATOR_MIN;
	}
	i_term = (int16_t)( integAcc / INTEGRATOR_SCALE );

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



#endif

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

#ifdef PID_DIRECT
 
int16_t dbg_PID_p_term;
int16_t dbg_PID_d_term;
int16_t dbg_PID_i_term;
int16_t dbg_PID_output;

static filter8bit_core_t dterm_filter_core;	// PID d-term filter core
static int16_t pid_dterm_buffer[4];			// PID d-term buffer	
static int8_t dterm_coeffs[] = {64,66,64,59};
//static int8_t dterm_coeffs[] = {60,64,67,69,67,64,60,57};

static uint16_t lastProcessValue;
static int32_t integAcc;


void initPID(uint16_t processValue)
{
	uint8_t i;
	dterm_filter_core.n = 4;
	dterm_filter_core.dc_gain = 25;
	//dterm_filter_core.n = 8;
	//dterm_filter_core.dc_gain = 51;
	dterm_filter_core.coeffs = dterm_coeffs;

	for (i=0;i<4;i++)
	{
		pid_dterm_buffer[i] = 0;
	}
	lastProcessValue = processValue;
	integAcc = 0;

	///////
	for (i = 0; i < MEDIAN_N; i++)
		mbuf[i] = 0;
}						
 
// Nice new model
uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t error, p_term, i_term, d_term, temp;
	//////
	int32_t integ_max;
	
	
	static int16_t error_p;

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
	if (error > INTEGRATOR_SOFT_RANGE)
		integ_max = 0;
	else if (error < 0)
		integ_max = INTEGRATOR_MAX;
	else
	{
		integ_max = (INTEGRATOR_SOFT_RANGE - (int32_t)error) * INTEGRATOR_SOFT_K;

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
	//push_i16(mbuf, MEDIAN_N, lastProcessValue - processValue);
	//d_term = get_median_i16(mbuf, MEDIAN_N);
	d_term = lastProcessValue - processValue;
	d_term = Kd * d_term;

	if (d_term > DIFF_MAX)
		d_term = DIFF_MAX;
	else if (d_term < DIFF_MIN)
		d_term = DIFF_MIN;

	lastProcessValue = processValue;
	
	//--------- Summ terms -----------//
	temp = (int16_t)( ((int32_t)p_term + (int32_t)i_term + (int32_t)d_term) / SCALING_FACTOR );
	
	if (temp > PID_OUTPUT_MAX)
	{
		temp = PID_OUTPUT_MAX;
	}
	else if (temp < PID_OUTPUT_MIN)
	{
		temp = PID_OUTPUT_MIN;
	}
	
	error_p = error;
	
	//------- Debug --------//
	dbg_PID_p_term = p_term;
	dbg_PID_d_term = d_term;
	dbg_PID_i_term = i_term;
	dbg_PID_output = temp;
	
	
	return (uint8_t)temp;
	
}

/*
int32_t integ_soft_k;

// Sets maximum integrator value for particular temperature setting point in order to reduce wind-up
// Argument is Cesius degree
// Call this function every time when the set point is changed and once during initialization
void setIntegratorLimit(uint8_t set_temp)
{
	uint8_t lim_percent;
	if (set_temp < 60)
		lim_percent = 20;
	else
		lim_percent = set_temp / 3;	// 20% for 60C, 30% for 90C, about 53% for 160C

	integ_soft_k = ((int32_t)lim_percent * 10000) / INTEGRATOR_SOFT_RANGE;
}
*/

/*
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
*/




#endif


