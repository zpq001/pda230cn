/*
 * soft_timer.h
 *
 * Created: 22.04.2013 12:08:37
 *  Author: Avega
 */
 
 

typedef struct {
	uint8_t		Enabled	: 1;
	uint8_t		RunOnce : 1;
	uint8_t		FA_TGL	: 1;		// (Timer == CompA) ? (^=1)
	uint8_t		FA_EQ	: 1;		// (Timer == CompA) ? 1 : 0
	uint8_t		FA_GE	: 1;		// (Timer >= CompA) ? 1 : 0
	uint8_t		FTop	: 1;		// (Timer >= Top) ? 1 : 0
	uint8_t		FOvfl	: 1;		// (Timer >= Top) ? 1 			<- sticky
	
	uint8_t		Timer;
	uint8_t		Top;
	uint8_t		CompA;
} SoftTimer8b_t;
 
 
 void processSoftTimer8b(SoftTimer8b_t *tmr);
 
 
 
 