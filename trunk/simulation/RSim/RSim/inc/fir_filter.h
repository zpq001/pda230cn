/*
 * fir_filter.h
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 
 
 typedef struct {
	uint8_t n;
	uint16_t dc_gain;
	int8_t* coeffs;
} filter8bit_core_t;



int16_t fir_i16_i8(int16_t new_sample, int16_t *samples, filter8bit_core_t* iir_core);







