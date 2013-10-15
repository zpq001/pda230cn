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
#define LONG_PRESS_DELAY	12	// After this delay button release will be considered as a long press

/*
typedef struct {
	uint8_t raw_state;			// Non-processed button state
	uint8_t action_down;		// Bit is set once when button is pressed
	uint8_t action_rep;			// Bit is set once when button is pressed. Bit is set continuously after REPEAT_DELAY until button is released
	uint8_t action_up_short;	// Bit is set once when button is released before LONG_PRESS_DELAY
	uint8_t action_long;		// Bit is set once when button is pressed for LONG_PRESS_DELAY (or longer)
} buttons_t;
*/

// BD = Button Down, the bit is set when button is pressed down
// BR = Button Repeat, bit is set when button is pressed continuously
// BS = Button Short, bit is set when button is released soon after press
// BL = Button Long, bit is set when button is released after long period of time


#define BD_MENU		0x20
#define BD_UP		0x40
#define BD_DOWN		0x80
#define BD_ROTFWD	0x04
#define BD_ROTREV	0x10
#define BD_HEATCTRL	0x01
#define BD_CYCLE	0x08

#define BS_MENU		0x0100	// short menu button press
#define BL_MENU		0x0200	// long menu button press 
#define BR_UP		0x0400
#define BR_DOWN		0x0800

#define BS_HEATCTRL	0x1000
#define BL_HEATCTRL	0x2000

/*
// Button signals
#define LONG_PRESS_EVENT	0x01	// set and cleared
#define LONG_PRESS_FLAG		0x02	// set stable
*/

extern uint16_t button_state;

extern uint8_t button_action_down;
extern uint8_t button_action_rep;
extern uint8_t button_action_up_short;
//extern uint8_t button_action_up_long;
extern uint8_t button_action_long;

//extern uint8_t button_signal;


void process_buttons();


#endif /* BUTTONS_H_ */