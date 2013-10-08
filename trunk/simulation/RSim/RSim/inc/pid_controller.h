/*
 * pid_controller.h
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 


//------------------------------------------//
// PID controller settings
//------------------------------------------//

// PID mode bits
#define PID_ENABLED				0x01
#define PID_RESET_INTEGRATOR	0x04


// Nice new model
//--------------------------//
// PID controller settings:
// Proportional
#define Kp	43				// Restrictions:
#define PROP_MAX 15000		//	PROP_MAX <= INT16_MAX (type of p_term)
#define PROP_MIN -15000		//	PROP_MAX / SCALING_FACTOR >= PID_OUTPUT_MAX

// Integral term is computed as: i_term = (integAcc += Ki * error)/INTEGRATOR_SCALE;
// integAcc is limited by INTEGRATOR_MAX and INTEGRATOR_MIN
// PID input: 1 count ~ 0.125 Celsius degree (see adc.c)
// Restrictions:
//	 INTEGRATOR_MAX / INTEGRATOR_SCALE <= INT16_MAX (type of i_term)
//	(INTEGRATOR_MAX / INTEGRATOR_SCALE) / SCALING_FACTOR <= PID_OUTPUT_MAX
#define Ki	35 
#define INTEGRATOR_MAX (50 * 10000L)
#define INTEGRATOR_MIN 0
#define INTEGRATOR_SCALE 100 	
// Intergrator limit function parameters
#define INTEGRATOR_SOFT_LIMIT
#define INTEGRATOR_SOFT_RANGE	(PROP_MAX / Kp)		// Start integrating when error becomes such that p_term is less than PROP_MAX
//#define INTEGRATOR_SOFT_MAX		(30 * 10000L)		// Intergator maximum when error = 0		
//#define INTEGRATOR_SOFT_K	(INTEGRATOR_SOFT_MAX / INTEGRATOR_SOFT_RANGE)

// Differential
#define Kd  400
#define DIFF_MAX	3000
#define DIFF_MIN	-3000

// Common scaling for summ of all terms
#define SCALING_FACTOR	100

// Output limits
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 100
//------------------------------------------//




extern int16_t dbg_PID_p_term;
extern int16_t dbg_PID_d_term;
extern int16_t dbg_PID_i_term;
extern int16_t dbg_PID_output;

void setPIDIntegratorLimit(uint8_t set_temp);
uint8_t processPID(uint16_t setPoint, uint16_t processValue, uint8_t mode);





