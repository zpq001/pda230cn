

#include "stdint.h"
#include "adc_func.h"


int32_t k_norm;					// integer, scaled by COEFF_SCALE
int32_t offset_norm;			// integer, scaled by COEFF_SCALE

float k_norm_f;					
float offset_norm_f;

cParams_t cp;




void calculateCoeffs_float(void)
{
	k_norm_f = ((float)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE) / ((float)(cp.cpoint2_adc - cp.cpoint1_adc));
	offset_norm_f = (float)cp.cpoint1 * COEFF_SCALE - (float)cp.cpoint1_adc * k_norm_f;
}


float conv_ADC_to_Celsius_float(uint16_t adc_value)
{	
	return (float)(((float)adc_value * k_norm_f + offset_norm_f) / (COEFF_SCALE));
}

float conv_Celsius_to_ADC_float(int16_t degree_value)
{
	//degree_value += 1;
	return (float)(((float)degree_value * COEFF_SCALE - offset_norm) / k_norm);
}

//----------------------------------------//
//----------------------------------------//
//----------------------------------------//

void calculateCoeffs(void)
{
	//k_norm = ((int32_t)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE) / ((int32_t)(cp.cpoint2_adc - cp.cpoint1_adc));	// Truncate
	int16_t temp = cp.cpoint2_adc - cp.cpoint1_adc;
	k_norm = ((int32_t)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE + (int32_t)(temp>>1)) / ((int32_t)temp);				// Round
	offset_norm = (int32_t)cp.cpoint1 * COEFF_SCALE - (int32_t)cp.cpoint1_adc * k_norm;
}


int16_t conv_ADC_to_Celsius(uint16_t adc_value)
{	
	//return (int16_t)(((int32_t)adc_value * k_norm + offset_norm) / (COEFF_SCALE));					// Truncate
	return (int16_t)(((int32_t)adc_value * k_norm + offset_norm + (COEFF_SCALE>>1)) / (COEFF_SCALE));	// Round
}

uint16_t conv_Celsius_to_ADC(int16_t degree_value)
{
	//degree_value += 1;
	//return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm) / k_norm);			// Truncate
	return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm + (k_norm>>1)) / k_norm);	// Round
}



