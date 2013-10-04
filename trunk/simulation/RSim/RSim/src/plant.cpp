

#include "stdint.h"
#include "plant.h"
#include "simulation.h"
#include "iir_filter.h"




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
#define plant_NCoef 4
static double plant_y[plant_NCoef+1]; //output samples
static double plant_x[plant_NCoef+1]; //input samples

double plant_ACoef[plant_NCoef+1] = {
        0.00000000067117469390,
        0.00000000268469877561,
        0.00000000402704816342,
        0.00000000268469877561,
        0.00000000067117469390
    };

    double plant_BCoef[plant_NCoef+1] = {
        1.00000000000000000000,
        -3.97263546992882950000,
        5.91828019705741950000,
        -3.91865100043767800000,
        0.97300628517191312000
    };

static iir_double_core_t plant_iir_core;



//----------------------------//
/*  good
#define eff_NCoef 4
static double eff_y[eff_NCoef+1]; //output samples
static double eff_x[eff_NCoef+1]; //input samples

double eff_ACoef[eff_NCoef+1] = {
        0.00000000005929847418,
        0.00000000023719389671,
        0.00000000035579084507,
        0.00000000023719389671,
        0.00000000005929847418
    };

    double eff_BCoef[eff_NCoef+1] = {
        1.00000000000000000000,
        -3.99397387738456230000,
        5.98197913518426280000,
        -3.98203645370192310000,
        0.99403119633051884000
    };
*/

/* too big delay
#define eff_NCoef 3
static double eff_y[eff_NCoef+1]; //output samples
static double eff_x[eff_NCoef+1]; //input samples

double eff_ACoef[eff_NCoef+1] = {
        0.00000000931329148875,
        0.00000002793987446626,
        0.00000002793987446626,
        0.00000000931329148875
    };

    double eff_BCoef[eff_NCoef+1] = {
        1.00000000000000000000,
        -2.99688763931455470000,
        2.99378750508951930000,
        -0.99689985056499619000
    };
*/

#define eff_NCoef 3
static double eff_y[eff_NCoef+1]; //output samples
static double eff_x[eff_NCoef+1]; //input samples

double eff_ACoef[eff_NCoef+1] = {
        0.00000001159228067557,
        0.00000003477684202670,
        0.00000003477684202670,
        0.00000001159228067557
    };

    double eff_BCoef[eff_NCoef+1] = {
        1.00000000000000000000,
        -2.99584690314718530000,
        2.99171554448822490000,
        -0.99586860530641597000
    };

static iir_double_core_t eff_iir_core;
//--------------------------//
	
static double plantAmbient;
static double plantState;
static double plantStateFiltered;

//static double k_amb = 0.07; // both good and big delay
static double k_amb = 0.1; 
//static double k_eff = 0.25; // good
//static double k_eff = 0.113; // too big delay
static double k_eff = 0.218;
static double timeConst = 0.0055 * TIMESTEP;




void initPlant(double ambient, double state)
{
	plantAmbient = ambient;
	plantState = state;
	// Initialize plant filter
	plant_iir_core.NCoef = plant_NCoef;
	plant_iir_core.ACoef_p = plant_ACoef;
	plant_iir_core.BCoef_p = plant_BCoef;
	plant_iir_core.x_p = plant_x;
	plant_iir_core.y_p = plant_y;
	iir_double_init(state,&plant_iir_core);

	eff_iir_core.NCoef = eff_NCoef;
	eff_iir_core.ACoef_p = eff_ACoef;
	eff_iir_core.BCoef_p = eff_BCoef;
	eff_iir_core.x_p = eff_x;
	eff_iir_core.y_p = eff_y;
	iir_double_init(0,&eff_iir_core);
}

void processPlant(double effect)
{
	// Simple 1st order model
	double effect_filtered = iir_double(effect, &eff_iir_core);
	plantState += (k_amb * (plantAmbient - plantState) + k_eff * effect_filtered ) * timeConst;
	plantStateFiltered = iir_double(plantState, &plant_iir_core);

}


double getPlantState(void)
{
	return plantStateFiltered;
}








