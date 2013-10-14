/*
 * adc.h
 *
 * Created: 31.03.2013 21:15:40
 *  Author: Avega
 */ 


#ifndef ADC_H_
#define ADC_H_

//#define ADC_OVERSAMPLE_RATE		4
#define ADC_BUFFER_LENGTH 		32
#define COEFF_SCALE				10000L


// adc_status bits:
#define SENSOR_ERROR_NO_PRESENT	(1<<0)
#define SENSOR_ERROR_SHORTED	(1<<1)

// adc error borders:
#define ADC_LOW_CORRECT		50
#define ADC_HIGH_CORRECT	1000


extern uint16_t adc_normalized;
extern int16_t adc_celsius;
extern uint16_t adc_filtered;
extern uint8_t adc_status;


void update_normalized_adc(void);
void update_Celsius(void);
void calculateCoeffs(void);
int16_t conv_ADC_to_Celsius(uint16_t adc_value);
uint16_t conv_Celsius_to_ADC(int16_t degree_value);
void update_CalibrationPoint(uint8_t point_number, uint8_t new_val_celsius);



#endif /* ADC_H_ */