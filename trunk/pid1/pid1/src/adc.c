/*
 * adc.c
 *
 * Created: 31.03.2013 21:15:32
 *  Author: Avega
 */ 


#include "compilers.h"
#include "port_defs.h"
#include "adc.h"
#include "control.h"



uint16_t adc_normalized;			// normalized (used for calibration) ADC value
uint16_t adc_celsius;				// celsius degree value (used for indication / calibration)


uint16_t raw_adc_buffer[ADC_BUFFER_LENGTH];	// Raw ADC ring buffer
RingBufU16_t ringBufADC = {
	.length = ADC_BUFFER_LENGTH,
	.data = raw_adc_buffer,
	.stat = RINIT
};

// Internal variables
static int32_t k_norm;				// integer, scaled by COEFF_SCALE
static int32_t offset_norm;			// integer, scaled by COEFF_SCALE

// Ring buffer functions
void addToRingU16(RingBufU16_t* bptr, uint16_t sample);
uint16_t getNormalizedRingU16(RingBufU16_t* bptr);

//---------------------------------------------//
//---------------------------------------------//


uint16_t conv_ADC_to_Celsius(uint16_t adc_value)
{	
	return (uint16_t)(((int32_t)adc_value * k_norm + offset_norm) / (COEFF_SCALE));
}

uint16_t conv_Celsius_to_ADC(uint16_t degree_value)
{	
	return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm) / k_norm);
}

void calculateCoeffs(void)
{
	k_norm = ((int32_t)(cpoint2 - cpoint1) * COEFF_SCALE) / ((int32_t)(cpoint2_adc - cpoint1_adc));
	offset_norm = (int32_t)cpoint1 * COEFF_SCALE - (int32_t)cpoint1_adc * k_norm;
}


void update_normalized_adc()
{
	// Disable interrupts from ADC - to save data integrity
	ACSR &= ~(1<<ACIE);	
	// Get normalized mean window summ
	adc_normalized = (uint16_t)getNormalizedRingU16(&ringBufADC);
	// Enable interrupts from ADC
	ACSR |= (1<<ACIE);
	// Convert to Celsius degree
	adc_celsius = conv_ADC_to_Celsius(adc_normalized);
}





ISR(ADC_vect)
{
	// Get new sample
	uint16_t new_sample = 1024 - ADC;	
	
	// Filter by mean window
	addToRingU16(&ringBufADC, new_sample);
}	








//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//
// Ring buffer implementation
//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//

// Ring buffer main function - add new data and update summ
void addToRingU16(RingBufU16_t* bptr, uint16_t sample)
{
	if (bptr->stat == RNORM)
	{
		bptr->summ -= bptr->data[bptr->curr_pos];
	}
	else
	{
		bptr->curr_pos = 0;
		bptr->summ = 0;
	}
	do	
	{
		bptr->data[bptr->curr_pos++] = sample;
		bptr->summ += sample;
		if (bptr->curr_pos == bptr->length)	
		{	
			bptr->curr_pos = 0;	
			bptr->stat = RNORM;
		}
	} 
	while (bptr->stat != RNORM);
}

// Get ring buffer normalized value
uint16_t getNormalizedRingU16(RingBufU16_t* bptr)
{
	return 	bptr->summ / bptr->length;
}



