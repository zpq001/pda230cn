/*
 * adc.h
 *
 * Created: 31.03.2013 21:15:40
 *  Author: Avega
 */ 


#ifndef ADC_H_
#define ADC_H_

// ----- Ring buffer ----- //
#define RINIT	0x01
#define RNORM	0x00

typedef struct {
	const uint8_t length;
	uint16_t *data;
	uint32_t summ;
	uint8_t curr_pos;
	uint8_t stat;
} RingBufU16_t;




#define ADC_BUFFER_LENGTH 20
#define COEFF_SCALE	1000


extern uint16_t adc_normalized;
extern uint16_t adc_celsius;
extern RingBufU16_t ringBufADC;
extern uint16_t PIDsampledADC;
extern uint16_t adc_oversampled;

void calculateCoeffs(void);
uint16_t conv_ADC_to_Celsius(uint16_t adc_value);
uint16_t conv_Celsius_to_ADC(uint16_t degree_value);
void update_normalized_adc(void);
void update_Celsius(void);

void addToRingU16(RingBufU16_t* bptr, uint16_t sample);
uint16_t getNormalizedRingU16(RingBufU16_t* bptr);

#endif /* ADC_H_ */