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



uint16_t adc_filtered_value;		// summ of the ring buffer counts (normalized)
uint16_t adc_filtered_celsius;		// celsius degree value (used for indication)


static int32_t k_norm;				// integer, scaled by COEFF_SCALE
static int32_t offset_norm;			// integer, scaled by COEFF_SCALE

static uint16_t adc_sample_buffer[ADC_BUFFER_LENGTH];
static int8_t adc_buffer_pos = -1;



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
	k_norm = ((int32_t)(cpoint1 - cpoint2) * COEFF_SCALE) / ((int32_t)(cpoint1_adc - cpoint2_adc));
	offset_norm = (int32_t)cpoint1 * COEFF_SCALE - (int32_t)cpoint1_adc * k_norm;
}


void update_filtered_adc()
{
	uint8_t i;
	uint32_t filtered_value = 0;
	// Disable interrupts from ADC - to save data integrity
	ACSR &= ~(1<<ACIE);	
	// Count up
	for(i=0;i<ADC_BUFFER_LENGTH;i++)
	{
		filtered_value += adc_sample_buffer[i];
	}	
	// Enable interrupts from ADC
	ACSR |= (1<<ACIE);
	// Normalize ADC filtered value
	adc_filtered_value = (uint16_t)(filtered_value / ADC_BUFFER_LENGTH);
	// Convert to Celsius degree
	adc_filtered_celsius = conv_ADC_to_Celsius(adc_filtered_value);
}





ISR(ADC_vect)
{
	uint16_t new_sample = ADC;
	
	if (adc_buffer_pos < 0)
	{
		// First call to the function, fill whole buffer with current sample
		for (adc_buffer_pos = ADC_BUFFER_LENGTH-1; adc_buffer_pos > 0; adc_buffer_pos--)
			adc_sample_buffer[adc_buffer_pos] = new_sample;
	}
	else
	{
		// Normal call
		adc_sample_buffer[adc_buffer_pos] = new_sample;
		adc_buffer_pos = (adc_buffer_pos == ADC_BUFFER_LENGTH - 1) ? 0 : adc_buffer_pos + 1;
	}		
}	





