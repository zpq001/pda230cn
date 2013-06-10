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
uint16_t adc_celsius;				// Celsius degree value (used for indication / calibration)
uint16_t adc_oversampled;
uint16_t adc_filtered;

uint16_t raw_adc_buffer[ADC_BUFFER_LENGTH];	// Raw ADC ring buffer
RingBufU16_t ringBufADC = {
	.length = ADC_BUFFER_LENGTH,
	.data = raw_adc_buffer,
	.stat = RINIT
};


uint16_t filter_buffer[20];

filter8bit_core_t fir_filter_rect = {
	.coeffs = {
		   11,
           21,
           33,
           44,
           55,
           65,
           73,
           79,
           83,
           84,
           83,
           79,
           73,
           65,
           55,
           44,
           33,
           21,
           11,
            2
	},
	.n = 20,
	.dc_gain = 1014
};


// Internal variables
static int32_t k_norm;				// integer, scaled by COEFF_SCALE
static int32_t offset_norm;			// integer, scaled by COEFF_SCALE

// Ring buffer functions
void addToRingU16(RingBufU16_t* bptr, uint16_t sample);
uint16_t getNormalizedRingU16(RingBufU16_t* bptr);

//---------------------------------------------//
//---------------------------------------------//

/*
	Oversampled ADC:
		145 Celsius ->	2100 
		25 Celsius	->	765
*/


uint16_t conv_ADC_to_Celsius(uint16_t adc_value)
{	
	return (uint16_t)(((int32_t)adc_value * k_norm + offset_norm) / (COEFF_SCALE));
}

uint16_t conv_Celsius_to_ADC(uint16_t degree_value)
{
	degree_value += 1;
	return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm) / k_norm);
}

void calculateCoeffs(void)
{
	k_norm = ((int32_t)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE) / ((int32_t)(cp.cpoint2_adc - cp.cpoint1_adc));
	offset_norm = (int32_t)cp.cpoint1 * COEFF_SCALE - (int32_t)cp.cpoint1_adc * k_norm;
}


void update_normalized_adc()
{
	// Disable interrupts from ADC - to save data integrity
	ADCSRA &= ~(1<<ADIE);	
	// Get normalized mean window summ
	adc_normalized = (uint16_t)getNormalizedRingU16(&ringBufADC);
	adc_oversampled = ringBufADC.summ >> 2;
	// Filter
	adc_filtered = fir_i16_i8(adc_oversampled, filter_buffer, &fir_filter_rect);	

	// Enable interrupts from ADC
	ADCSRA |= (1<<ADIE);
}

void update_Celsius(void)
{
	// Convert to Celsius degree
	adc_celsius = conv_ADC_to_Celsius(adc_normalized);
}





ISR(ADC_vect)
{
	// Get new sample
	uint16_t new_sample = 1024 - ADC;	
	// Add new sample to the ring buffer
	addToRingU16(&ringBufADC, new_sample);
}	



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



