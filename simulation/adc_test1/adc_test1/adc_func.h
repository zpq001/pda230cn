

typedef struct  
{
	uint16_t cpoint1;
	uint16_t cpoint2;
	uint16_t cpoint1_adc;
	uint16_t cpoint2_adc;
} cParams_t;


#define COEFF_SCALE				10000L

extern cParams_t cp;

extern int32_t k_norm;					// integer, scaled by COEFF_SCALE
extern int32_t offset_norm;			// integer, scaled by COEFF_SCALE
extern float k_norm_f;					
extern float offset_norm_f;

void calculateCoeffs(void);
int16_t conv_ADC_to_Celsius(uint16_t adc_value);
uint16_t conv_Celsius_to_ADC(int16_t degree_value);

void calculateCoeffs_float(void);
float conv_ADC_to_Celsius_float(uint16_t adc_value);
float conv_Celsius_to_ADC_float(int16_t degree_value);