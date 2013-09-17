/*
 * pid_controller.h
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 
 
 
 //--------------------------//
// PID controller settings
// Proportional
#define Kp		20
#define PROP_MAX 2000		// Limits are pretty arbitrary
#define PROP_MIN -2000

// Integral term is computed as: i_term = (integAcc += Ki * error)/INTEGRATOR_SCALE;
// integAcc is limited by INTEGRATOR_MAX and INTEGRATOR_MIN
#define Ki 4
#define INTEGRATOR_MAX 20000
#define INTEGRATOR_MIN 0
#define INTEGRATOR_SCALE 40

// Differential
#define Kd		80

// Common scaling for summ of all terms
#define SCALING_FACTOR	5
// Output limits
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 100
//--------------------------//


extern int16_t dbg_PID_p_term;
extern int16_t dbg_PID_d_term;
extern int16_t dbg_PID_i_term;
extern int16_t dbg_PID_output;

void initPID(uint16_t processValue);
uint8_t processPID(uint16_t setPoint, uint16_t processValue);




