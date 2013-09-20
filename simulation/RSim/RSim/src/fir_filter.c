/*
 * fir_filter.c
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 
 
#include "compilers.h"
#include "fir_filter.h"
 

//---------------------------------------------//
// FIR digital filter
// Samples: signed, 16-bit
// Coeffs:  signed, 8-bit
//---------------------------------------------//
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




