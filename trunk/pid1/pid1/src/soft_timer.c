/*
 * soft_timer.c
 *
 * Created: 22.04.2013 12:06:42
 *  Author: Avega
 */ 
 
 #include "compilers.h"
 #include "soft_timer.h"
 
 
 
 
 void processSoftTimer8b(SoftTimer8b_t *tmr)
 {
	 uint8_t newTimerVal;

	 if (!tmr->Enabled)
		return;
	 
	 tmr->FA_EQ = 0;
	 tmr->FA_GE = 0;
	 tmr->FTop = 0;
	 
	 if (tmr->Timer >= tmr->Top)
	 {
		 if (tmr->RunOnce)
		 {
			 tmr->Enabled = 0;
		 }
		 tmr->FTop = 1;
		 tmr->FOvfl = 1;
		 newTimerVal = 0;
	 }
	 else
	 {
		 newTimerVal = tmr->Timer + 1;
	 }
	 
	 if (tmr->Timer == tmr->CompA)
	 {
		 tmr->FA_EQ = 1;
		 tmr->FA_GE = 1;
		 tmr->FA_TGL = !tmr->FA_TGL;
	 }
	 else if (tmr->Timer >= tmr->CompA)
	 {
		 tmr->FA_GE = 1;
	 }
	 
	 tmr->Timer = newTimerVal; 
 }
 
 