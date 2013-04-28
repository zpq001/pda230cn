/*
 * adc.h
 *
 * Created: 31.03.2013 21:15:40
 *  Author: Avega
 */ 


#ifndef ADC_H_
#define ADC_H_

#define ADC_BUFFER_LENGTH 8


extern uint16_t adc_filtered_value;

void calculateCoeffs(uint16_t deg_a, uint16_t adc_a, uint16_t deg_b, uint16_t adc_b);
uint16_t conv_ADC_to_Celsius(uint16_t adc_value);
void update_filtered_adc();


#endif /* ADC_H_ */