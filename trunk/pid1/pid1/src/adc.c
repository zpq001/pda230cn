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
#include "fir_filter.h"

/*
	ADC & PID data types:
	
	Temperature setup (Celsius):		uint8_t
	Calibration points (Celsius):		uint8_t
	Measured temperature (Celsius):		int16_t
	Measured temperature (normalized):	uint16_t
	
	PID input: f(adc_filtered) => 	uint16_t
	PID set point. f(setup_temp_value) => uint16_t
*/



uint16_t adc_normalized;			// normalized (used for calibration) ADC value
int16_t adc_celsius;				// Celsius degree value (used for indication / calibration)
uint16_t adc_oversampled;			// Oversampled and filtered ADC versions are used for PID
uint16_t adc_filtered;				//		control only
uint8_t adc_status;					// Sensor and ADC status

uint16_t raw_adc_buffer[ADC_BUFFER_LENGTH];	// Raw ADC buffer


int16_t filter_buffer[20];

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

//---------------------------------------------//
//---------------------------------------------//


int16_t conv_ADC_to_Celsius(uint16_t adc_value)
{	
	return (int16_t)(((int32_t)adc_value * k_norm + offset_norm) / (COEFF_SCALE));
}

uint16_t conv_Celsius_to_ADC(int16_t degree_value)
{
	degree_value += 1;
	return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm) / k_norm);
}

void calculateCoeffs(void)
{
	k_norm = ((int32_t)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE) / ((int32_t)(cp.cpoint2_adc - cp.cpoint1_adc));
	offset_norm = (int32_t)cp.cpoint1 * COEFF_SCALE - (int32_t)cp.cpoint1_adc * k_norm;
}

/*
	In order to increase ADC resolution oversampling is used.
	To get result with n extra bits the summ of 4^n samples must be divided by 2^n.
	If we want 12-bit result from 10-bit raw ADC, we should get 4^2 = 16 samples, summ them and
	divide by 2^2 = 4 (equal to right-shift by 2)
	
	adc_oversampled (adc_filtered) is about 8x celsius_degree - 
		~2x - hardware ADC scale
		4x - oversampling
*/


void update_normalized_adc()
{
	uint8_t i;
	uint16_t adc_raw_summ = 0;
	// Disable interrupts from ADC - to save data integrity
	ADCSRA &= ~(1<<ADIE);	
	// Get normalized mean window summ
	for (i=0;i<ADC_BUFFER_LENGTH;i++)
		adc_raw_summ += raw_adc_buffer[i];
	// Enable interrupts from ADC
	ADCSRA |= (1<<ADIE);
	
	adc_normalized = adc_raw_summ >> 5;		// ADC_BUFFER_LENGTH = 32 !
	adc_oversampled = adc_raw_summ >> 3;	// adc_oversampled is 4 times greater than adc_normalized
	// Filter
	adc_filtered = fir_i16_i8(adc_oversampled, filter_buffer, &fir_filter_rect);	
	// Check sensor
	adc_status = 0;
	if (adc_normalized < 50)
		adc_status |= SENSOR_ERROR_NO_PRESENT;
	else if (adc_normalized > 1000)
		adc_status |= SENSOR_ERROR_SHORTED;
}

void update_Celsius(void)
{
	// Convert to Celsius degree
	adc_celsius = conv_ADC_to_Celsius(adc_normalized);
}




// ADC conversion is started by system timer (Timer2 ISR) every 1 ms
ISR(ADC_vect)
{
	static uint8_t adc_buffer_pointer = ADC_BUFFER_LENGTH;
	// Get new sample
	uint16_t new_sample = 1024 - ADC;	
	// Add new sample to the buffer
	raw_adc_buffer[--adc_buffer_pointer] = new_sample;
	if (adc_buffer_pointer == 0)
		adc_buffer_pointer = ADC_BUFFER_LENGTH;
}	






