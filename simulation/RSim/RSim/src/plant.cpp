

#include "stdint.h"
#include "plant.h"
#include "simulation.h"





/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Butterworth
Filter order: 5
Sampling Frequency: 500 Hz
Cut Frequency: 1.000000 Hz
Coefficents Quantization: float

Z domain Zeros
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000

Z domain Poles
z = 0.985283 + j -0.004173
z = 0.985283 + j 0.004173
z = 0.994441 + j -0.015128
z = 0.994441 + j 0.015128
z = 0.999887 + j -0.000000
***************************************************************/
#define NCoef 4
static double y[NCoef+1]; //output samples
static double x[NCoef+1]; //input samples

double iir(double NewSample) {
    double ACoef[NCoef+1] = {
        0.00000000067117469390,
        0.00000000268469877561,
        0.00000000402704816342,
        0.00000000268469877561,
        0.00000000067117469390
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -3.97263546992882950000,
        5.91828019705741950000,
        -3.91865100043767800000,
        0.97300628517191312000
    };

    
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}
 

static double plantAmbient;
static double plantState;
static double plantStateFiltered;

static double k_amb = 0.07;
static double k_amb2 = 0;//0.00005;
static double k_eff = 0.55;
static double timeConst = 0.0055 * TIMESTEP;


void initPlant(double ambient, double state)
{
	int i;
	plantAmbient = ambient;
	plantState = state;
	// Initialize filter
	for (i=0;i<=NCoef;i++)
	{
		y[i] = plantState;
		x[i] = plantState;
	}
}

void processPlant(double effect)
{
	// Simple 1st order model
	plantState += (k_amb * (plantAmbient - plantState) + k_eff * effect + k_amb2 * (plantAmbient - plantState) * (plantAmbient - plantState) ) * timeConst;
	plantStateFiltered = iir(plantState);
	
}


double getPlantState(void)
{
	return plantStateFiltered;
}








