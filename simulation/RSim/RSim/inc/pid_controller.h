/*
 * pid_controller.h
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 

#define PID_DIRECT
//#define PID_INCREMENTAL
 
#ifdef PID_INCREMENTAL 

//--------------------------//
// PID controller settings

//--------------------------//


extern int16_t dbg_PID_p_term;
extern int32_t dbg_PID_d_term;
extern int32_t dbg_PID_i_term;
extern int16_t dbg_PID_output;

void initPID(uint16_t processValue);
uint8_t processPID(uint16_t setPoint, uint16_t processValue);

#endif

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////


#ifdef PID_DIRECT


//--------------------------//
// PI controller settings
// Proportional
#define Kp		30			// Restrictions:
#define PROP_MAX 10000		//	PROP_MAX <= INT16_MAX (type of p_term)
#define PROP_MIN -10000		//	PROP_MAX / SCALING_FACTOR = PID_OUTPUT_MAX

// Integral term is computed as: i_term = (integAcc += Ki * error)/INTEGRATOR_SCALE;
// integAcc is limited by INTEGRATOR_MAX and INTEGRATOR_MIN
// Restrictions:
//	 INTEGRATOR_MAX / INTEGRATOR_SCALE <= INT16_MAX (type of i_term)
//	(INTEGRATOR_MAX / INTEGRATOR_SCALE) / SCALING_FACTOR <= PID_OUTPUT_MAX
#define Ki		13
#define INTEGRATOR_MAX 400000
#define INTEGRATOR_MIN 0
#define INTEGRATOR_SCALE 40
#define INTEGRATOR_ENABLE_RANGE	160		// 1 count ~ 0.125 Celsius degree
#define INTEGRATOR_RANGE_LIMIT

// Differential
#define Kd		0

// Common scaling for summ of all terms
#define SCALING_FACTOR	100
// Output limits
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 100
//--------------------------//



/* 4 sec variant 
//--------------------------//
// PID controller settings
// Proportional
#define Kp	70 //	30			// Restrictions:
#define PROP_MAX 10000		//	PROP_MAX <= INT16_MAX (type of p_term)
#define PROP_MIN -10000		//	PROP_MAX / SCALING_FACTOR = PID_OUTPUT_MAX

// Integral term is computed as: i_term = (integAcc += Ki * error)/INTEGRATOR_SCALE;
// integAcc is limited by INTEGRATOR_MAX and INTEGRATOR_MIN
// Restrictions:
//	 INTEGRATOR_MAX / INTEGRATOR_SCALE <= INT16_MAX (type of i_term)
//	(INTEGRATOR_MAX / INTEGRATOR_SCALE) / SCALING_FACTOR <= PID_OUTPUT_MAX
#define Ki	60  //	13
#define INTEGRATOR_MAX 200000
#define INTEGRATOR_MIN 0
#define INTEGRATOR_SCALE 40
#define INTEGRATOR_ENABLE_RANGE	160		// 1 count ~ 0.125 Celsius degree
//#define INTEGRATOR_RANGE_LIMIT

// Differential
#define Kd 	300 //	0

// Common scaling for summ of all terms
#define SCALING_FACTOR	100
// Output limits
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 100
//--------------------------//
*/


extern int16_t dbg_PID_p_term;
extern int16_t dbg_PID_d_term;
extern int16_t dbg_PID_i_term;
extern int16_t dbg_PID_output;

void initPID(uint16_t processValue);
uint8_t processPID(uint16_t setPoint, uint16_t processValue);




#endif



