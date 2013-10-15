/*
 * buttons.h
 *
 * Created: 27.03.2013 22:30:52
 *  Author: Avega
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

// All delays in units of process_buttons() call period.
#define REPEAT_DELAY		6	// After this delay button press will be repeated continuously
#define LONG_PRESS_DELAY	12	// After this delay button press will be considered as long


typedef struct {
	uint8_t raw_state;			// Non-processed button state
	uint8_t action_down;		// Bit is set once when button is pressed
	uint8_t action_rep;			// Bit is set once when button is pressed. Bit is set continuously after REPEAT_DELAY until button is released
	uint8_t action_up_short;	// Bit is set once when button is released before LONG_PRESS_DELAY
	//uint8_t action_up_long;		// Bit is set once when button is released after LONG_PRESS_DELAY
	uint8_t action_long;		// Bit is set once when button is pressed for LONG_PRESS_DELAY (or longer)
} buttons_t;


// Button bits
#define BD_MENU		0x20
#define BD_UP		0x40
#define BD_DOWN		0x80
#define BD_ROTFWD	0x04
#define BD_ROTREV	0x10
#define BD_HEATCTRL	0x01
#define BD_CYCLE	0x08


extern buttons_t buttons;

void process_buttons();


#endif /* BUTTONS_H_ */