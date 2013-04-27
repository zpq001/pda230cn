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


// BD = Button Down, the bit is set when button is pressed down
// BR = Button Repeat, bit is set when button is pressed continuously
// BS = Button Short, bit is set when button is released soon after press
// BL = Button Long, bit is set when button is released after long period of time


#define BD_MENU		0x01
#define BD_UP		0x02
#define BD_DOWN		0x04
#define BD_ROTFWD	0x10
#define BD_ROTREV	0x20
#define BD_HEATCTRL	0x40
#define BD_CYCLE	0x80

#define BS_MENU		0x0100	// short menu button press
#define BL_MENU		0x0200	// long menu button press 
#define BR_UP		0x0400
#define BR_DOWN		0x0800

/*
// Button signals
#define LONG_PRESS_EVENT	0x01	// set and cleared
#define LONG_PRESS_FLAG		0x02	// set stable
*/

extern uint16_t button_state;

extern uint8_t button_action_down;
extern uint8_t button_action_rep;
extern uint8_t button_action_up_short;
extern uint8_t button_action_up_long;

//extern uint8_t button_signal;

void process_buttons();


#endif /* BUTTONS_H_ */