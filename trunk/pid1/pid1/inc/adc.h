/*
 * adc.h
 *
 * Created: 31.03.2013 21:15:40
 *  Author: Avega
 */ 


#ifndef ADC_H_
#define ADC_H_

#define ADC_BUFFER_LENGTH 8
#define COEFF_SCALE	1000


extern uint16_t adc_filtered_value;
extern uint16_t adc_filtered_celsius;

void calculateCoeffs(void);
uint16_t conv_ADC_to_Celsius(uint16_t adc_value);
uint16_t conv_Celsius_to_ADC(uint16_t degree_value);
void update_filtered_adc();


#endif /* ADC_H_ */