/*
 * pid_controller.h
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 
  #ifndef PID_CONTROLLER_H_
  #define PID_CONTROLLER_H_
  
  /*
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
 */
  
  //
  //--------------------------//
  // PID controller settings
  // Proportional
  #define Kp	40 //	30			// Restrictions:
  #define PROP_MAX 10000		//	PROP_MAX <= INT16_MAX (type of p_term)
  #define PROP_MIN -10000		//	PROP_MAX / SCALING_FACTOR = PID_OUTPUT_MAX

  // Integral term is computed as: i_term = (integAcc += Ki * error)/INTEGRATOR_SCALE;
  // integAcc is limited by INTEGRATOR_MAX and INTEGRATOR_MIN
  // Restrictions:
  //	 INTEGRATOR_MAX / INTEGRATOR_SCALE <= INT16_MAX (type of i_term)
  //	(INTEGRATOR_MAX / INTEGRATOR_SCALE) / SCALING_FACTOR <= PID_OUTPUT_MAX
  #define Ki	15  //	13
  #define INTEGRATOR_MAX 200000
  #define INTEGRATOR_MIN 0
  #define INTEGRATOR_SCALE 40

  //#define INTEGRATOR_RANGE_LIMIT
  //#define INTEGRATOR_ENABLE_RANGE	160		// 1 count ~ 0.125 Celsius degree


  #define INTEGRATOR_SOFT_LIMIT
  // 50C
  //#define INTEGRATOR_SOFT_RANGE	80
  //#define INTEGRATOR_SOFT_MAX		40000 // 10%
  // 90C
  #define INTEGRATOR_SOFT_RANGE	((PROP_MAX / 2) / Kp)
  //#define INTEGRATOR_SOFT_MAX		80000	// 20%
  #define INTEGRATOR_SOFT_MAX		40000	// 10%
  // 160C
  //#define INTEGRATOR_SOFT_RANGE	160
  //#define INTEGRATOR_SOFT_MAX		100000 // 25%

  // Differential
  #define Kd  0//300 //	0
  #define DIFF_MAX	2000
  #define DIFF_MIN	-2000

  // Common scaling for summ of all terms
  #define SCALING_FACTOR	100
  // Output limits
  #define PID_OUTPUT_MIN 0
  #define PID_OUTPUT_MAX 100
  //--------------------------//
 
 
/* 4 sec variant 15_7
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




#endif /* PID_CONTROLLER_H_ */