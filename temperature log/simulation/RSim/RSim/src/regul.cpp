



#include "stdint.h"
#include "regul.h"
#include "simulation.h"



filter8bit_core_t dterm_filter_core;	// PID d-term filter core
int16_t pid_dterm_buffer[8];			// PID d-term buffer

int16_t dbg_PID_p_term;
int16_t dbg_PID_d_term;
int16_t dbg_PID_i_term;
int16_t dbg_PID_output;



// FIR digital filter
int16_t fir_i16_i8(int16_t new_sample, int16_t *samples, filter8bit_core_t* iir_core)
{
	int32_t summ;
	uint8_t i;
	
	summ = new_sample * iir_core->coeffs[0];
	for (i=iir_core->n-1; i>0; i--)
	{
		samples[i] = samples[i-1];
		summ += (int32_t)samples[i] * iir_core->coeffs[i];
	}
	samples[0] = new_sample;
	return (int16_t)(summ / iir_core->dc_gain);
}



void initRegulator(void)
{
	int8_t dterm_coeffs[] = {64,66,64,59};
	dterm_filter_core.n = 4;
	dterm_filter_core.dc_gain = 256;
/*
	int8_t dterm_coeffs[] = {60,64,67,69,67,64,60,57};
	dterm_filter_core.n = 8;
	dterm_filter_core.dc_gain = 512;
*/
	dterm_filter_core.coeffs = dterm_coeffs;
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
	
	if (integAcc > 1500 )
	{
		integAcc = 1500;
	}
	else if (integAcc < 0)
	{
		integAcc = 0;
	}
	i_term = integAcc * Ki;
	i_term /= 20;
	
	
	//------ Calculate D term --------//
	// 12_6
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
	
	
	return (uint8_t)temp;
	
}
 

