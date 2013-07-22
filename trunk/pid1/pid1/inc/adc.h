/*
 * adc.h
 *
 * Created: 31.03.2013 21:15:40
 *  Author: Avega
 */ 


#ifndef ADC_H_
#define ADC_H_

// adc_status bits:
#define SENSOR_ERROR_NO_PRESENT	(1<<0)
#define SENSOR_ERROR_SHORTED	(1<<1)





typedef struct {
	uint8_t n;
	uint16_t dc_gain;
	int8_t coeffs[];
} filter8bit_core_t;



#define ADC_BUFFER_LENGTH 32
#define COEFF_SCALE	1000


extern uint16_t adc_normalized;
extern uint16_t adc_celsius;
extern uint16_t adc_oversampled;
extern uint16_t adc_filtered;
extern uint8_t adc_status;

void calculateCoeffs(void);
uint16_t conv_ADC_to_Celsius(uint16_t adc_value);
uint16_t conv_Celsius_to_ADC(uint16_t degree_value);
void update_normalized_adc(void);
void update_Celsius(void);

int16_t fir_i16_i8(int16_t new_sample, int16_t *samples, filter8bit_core_t* iir_core);


#endif /* ADC_H_ */