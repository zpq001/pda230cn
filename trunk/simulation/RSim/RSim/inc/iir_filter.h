

 typedef struct {
	int NCoef;
	double *ACoef_p;
	double *BCoef_p;
	double *x_p;
	double *y_p;
} iir_double_core_t;


 void iir_double_init(double value,  iir_double_core_t *fcore);
 double iir_double(double NewSample, iir_double_core_t *fcore);
