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



uint16_t adc_normalized;	// normalized ADC value (used for debug only)
int16_t adc_celsius;		// Celsius degree value (used for indication / calibration)
uint16_t adc_filtered;		// Oversampled and filtered ADC value, used for conversion to Celsius, calibration and PID
uint8_t adc_status;			// Sensor and ADC status


// Internal variables
static uint16_t raw_adc_buffer[ADC_BUFFER_LENGTH];	// Buffer for raw ADC samples
static int16_t filter_buffer[20];					// FIR filter buffer

static filter8bit_core_t fir_filter_rect = {
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


static int32_t k_norm;				// integer, scaled by COEFF_SCALE
static int32_t offset_norm;			// integer, scaled by COEFF_SCALE

//---------------------------------------------//
//---------------------------------------------//


int16_t conv_ADC_to_Celsius(uint16_t adc_value)
{	
	//return (int16_t)(((int32_t)adc_value * k_norm + offset_norm) / (COEFF_SCALE));					// Truncate
	return (int16_t)(((int32_t)adc_value * k_norm + offset_norm + (COEFF_SCALE>>1)) / (COEFF_SCALE));	// Round
}

uint16_t conv_Celsius_to_ADC(int16_t degree_value)
{
	//degree_value += 1;
	//return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm) / k_norm);				// Truncate
	return (uint16_t)(((int32_t)degree_value * COEFF_SCALE - offset_norm + (k_norm>>1)) / k_norm);	// Round
}

void calculateCoeffs(void)
{
	//k_norm = ((int32_t)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE) / ((int32_t)(cp.cpoint2_adc - cp.cpoint1_adc));	// Truncate
	int16_t temp = cp.cpoint2_adc - cp.cpoint1_adc;
	k_norm = ((int32_t)(cp.cpoint2 - cp.cpoint1) * COEFF_SCALE + (int32_t)(temp>>1)) / ((int32_t)temp);				// Round
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


// It is unsafe to enable/disable ADC interrupts using sbi/cbi instructions - 
// 		the interrupt request can be missed. 
// Since ADC setup (channels, etc) are constant,
// 	simple ADCSRA write with ADIE bit set or cleared is used instead.

void update_normalized_adc()
{
	uint8_t i;
	uint16_t adc_raw_summ = 0;
	uint16_t adc_oversampled;
	
	// Disable interrupts from ADC - to save data integrity
	ADCSRA = (1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0);
	
	// Get normalized mean window summ
	for (i=0;i<ADC_BUFFER_LENGTH;i++)
		adc_raw_summ += raw_adc_buffer[i];
	
	// Enable interrupts from ADC
	ADCSRA = (1<<ADEN | 1<<ADIE | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0);
	
	adc_normalized = adc_raw_summ >> 5;		// ADC_BUFFER_LENGTH = 32 !
	adc_oversampled = adc_raw_summ >> 3;	// adc_oversampled is 4 times greater than adc_normalized
	// Filter
	adc_filtered = fir_i16_i8(adc_oversampled, filter_buffer, &fir_filter_rect);	
	// Check sensor
	adc_status = 0;
	if (adc_normalized < ADC_LOW_CORRECT)
		adc_status |= SENSOR_ERROR_NO_PRESENT;
	else if (adc_normalized > ADC_HIGH_CORRECT)
		adc_status |= SENSOR_ERROR_SHORTED;
}

void update_Celsius(void)
{
	// Convert to Celsius degree
	adc_celsius = conv_ADC_to_Celsius(adc_filtered);
}

void update_CalibrationPoint(uint8_t point_number, uint8_t new_val_celsius)
{
	if (point_number == 1)
	{
		cp.cpoint1 = new_val_celsius;
		cp.cpoint1_adc = adc_filtered;
	}
	else if (point_number == 2)
	{
		cp.cpoint2 = new_val_celsius;
		cp.cpoint2_adc = adc_filtered;
	}
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






