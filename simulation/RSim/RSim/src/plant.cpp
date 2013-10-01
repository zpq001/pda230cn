

#include "stdint.h"
#include "plant.h"
#include "simulation.h"

 
 /**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Butterworth
Filter order: 2
Sampling Frequency: 500 Hz
Cut Frequency: 1.000000 Hz
Coefficents Quantization: float

Z domain Zeros
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000

Z domain Poles
z = 0.991114 + j -0.008807
z = 0.991114 + j 0.008807
***************************************************************/
#define NCoef 2

 static float y[NCoef+1]; //output samples
 static float x[NCoef+1]; //input samples

float iir(float NewSample) {
    float ACoef[NCoef+1] = {
        0.00000994703743556282,
        0.00001989407487112564,
        0.00000994703743556282
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -1.99111429220165360000,
        0.99115359586893537000
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
static double k_eff = 0.38;
static double timeConst = 0.007 * TIMESTEP;


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
	plantState += (k_amb * (plantAmbient - plantState) + k_eff * effect) * timeConst;
	plantStateFiltered = iir(plantState);
}


double getPlantState(void)
{
	return plantStateFiltered;
}








