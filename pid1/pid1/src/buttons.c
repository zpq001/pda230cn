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


// Global variable "raw_button_state" is updated in module "led_indic.h"		


uint16_t button_state = 0;	// global processed button state
//uint8_t button_signal = 0;

uint8_t button_action_down = 0;
uint8_t button_action_rep = 0;
uint8_t button_action_up_short = 0;
//uint8_t button_action_up_long = 0;
uint8_t button_action_long = 0;



void process_buttons()
{
	static uint8_t raw_current = 0;
	static uint8_t raw_current_inv = 0xFF;
	uint8_t raw_delayed;
	uint8_t raw_delayed_inv;
	static uint8_t press_timer = 0;
	uint8_t long_press_mask;
	uint8_t long_press_event_mask;
	
	raw_delayed = raw_current;
	raw_delayed_inv = raw_current_inv;
	raw_current = raw_button_state;
	raw_current_inv = ~raw_current;
	
	// Must be before press_timer update
	long_press_mask = (press_timer >= LONG_PRESS_DELAY) ? 0xFF : 0x00;
	
	if (raw_delayed != raw_current)
	{
		press_timer = 0;
	}
	else if (press_timer <= LONG_PRESS_DELAY)
	{
		press_timer++;
	}
	
	// Must after press_timer update
	long_press_event_mask = (press_timer == LONG_PRESS_DELAY) ? 0xFF : 0x00;
			
	button_action_down = raw_current & raw_delayed_inv;
			
	if (press_timer > REPEAT_DELAY)
		raw_delayed_inv |= 0xFF;
	
	button_action_rep = raw_current & raw_delayed_inv;
	
	button_action_up_short = raw_current_inv & raw_delayed & ~long_press_mask;
	//button_action_up_long  = raw_current_inv & raw_delayed & long_press_mask;
	button_action_long  = raw_current & long_press_event_mask;
	
	// Compose button state
	button_state = (button_action_down & (BD_MENU | BD_UP | BD_DOWN | BD_ROTFWD | BD_ROTREV | BD_HEATCTRL | BD_CYCLE));
	if (button_action_up_short & BD_MENU)
		button_state |= BS_MENU;
	//if (button_action_up_long & BD_MENU)
	if (button_action_long & BD_MENU)
		button_state |= BL_MENU;
	if (button_action_rep & BD_UP)
		button_state |= BR_UP;	
	if (button_action_rep & BD_DOWN)
		button_state |= BR_DOWN;

}

