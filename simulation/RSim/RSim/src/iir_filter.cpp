
#include "iir_filter.h"


void iir_double_init(double value,  iir_double_core_t *fcore) {
	int i;

	for (i = 0; i < fcore->NCoef; i++)
	{
		fcore->x_p[i] = value;
		fcore->y_p[i] = value;
	}
}


double iir_double(double NewSample, iir_double_core_t *fcore) {
    
    int n;

    //shift the old samples
    for(n=fcore->NCoef; n>0; n--) {
       fcore->x_p[n] = fcore->x_p[n-1];
       fcore->y_p[n] = fcore->y_p[n-1];
    }

    //Calculate the new output
    fcore->x_p[0] = NewSample;
    fcore->y_p[0] = fcore->ACoef_p[0] * fcore->x_p[0];
    for(n=1; n<=fcore->NCoef; n++)
        fcore->y_p[0] += fcore->ACoef_p[n] * fcore->x_p[n] - fcore->BCoef_p[n] * fcore->y_p[n];
    
    return fcore->y_p[0];
}


