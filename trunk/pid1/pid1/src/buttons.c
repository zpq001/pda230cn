/*
 * buttons.c
 *
 *	Module for button functions
 *
 * Created: 27.03.2013 22:30:40
 *  Author: Avega
 */ 

#include "compilers.h"
#include "buttons.h"
#include "led_indic.h"


// Global variable "raw_button_state" is updated in module "led_indic.c"		

buttons_t buttons;		// Processed result


void process_buttons()
{
	static uint8_t raw_current = 0;
	uint8_t raw_current_inv;
	uint8_t raw_delayed;
	uint8_t raw_delayed_inv;
	static uint8_t press_timer = 0;		
	uint8_t event_release_short = 0x00;
	//uint8_t event_release_long = 0x00;
	uint8_t state_repeat = 0x00;
	uint8_t event_hold = 0x00;
	buttons_t *buttons_p = &buttons;
	
	
	// Get the delayed versions of raw button state
	raw_delayed = raw_current;
	raw_delayed_inv = ~raw_current;
	// Update current
	raw_current = raw_button_state;
	raw_current_inv = ~raw_current;
	
	
	// If some button is pressed or released
	if (raw_delayed != raw_current)
	{
		if (press_timer < LONG_PRESS_DELAY)
			event_release_short = 0xFF;			// Button release short
		//else
			//event_release_long = 0xFF;			// Button release long
			
		// Reset timer	
		press_timer = 0;
	}
	else
	{	
		// Increment timer
		if (press_timer != (LONG_PRESS_DELAY + 1))
		{
			press_timer++;
		}
		
		if (press_timer >= REPEAT_DELAY)
		{
			state_repeat = 0xFF;
		}
		if (press_timer == LONG_PRESS_DELAY)
		{
			event_hold = 0xFF;
		}
	}
	
	
	PRELOAD("z",buttons_p);
	buttons_p->raw_state = raw_current;
	buttons_p->action_down = raw_current & raw_delayed_inv;
	buttons_p->action_rep = raw_current & (raw_delayed_inv | state_repeat);
	buttons_p->action_up_short = raw_current_inv & raw_delayed & event_release_short;
	//buttons_p->action_up_long = raw_current_inv & raw_delayed & event_release_long;
	buttons_p->action_long = raw_current & event_hold;
}


