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
// PID controller settings
// Proportional
#define Kp	5	//15
#define PROP_MAX 500		
#define PROP_MIN -500

// Integral term is computed as: i_term = (integAcc += Ki * error)/INTEGRATOR_SCALE;
// integAcc is limited by INTEGRATOR_MAX and INTEGRATOR_MIN
#define Ki 3//7
#define INTEGRATOR_MAX 20000
#define INTEGRATOR_MIN 0
#define INTEGRATOR_SCALE 40
#define INTEGRATOR_ENABLE_RANGE	120	// 40(80) is about 10 C degree
#define INTEGRATOR_RANGE_LIMT

// Differential
#define Kd	0//	25

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




#endif



