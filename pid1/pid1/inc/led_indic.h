/*
 * led_indic.h
 *
 * Created: 24.03.2013 18:23:27
 *  Author: Avega
 */ 


#ifndef LED_INDIC_H_
#define LED_INDIC_H_


/************************************************
*					Settings					*
************************************************/

#define LED_BUFFER_LENGTH	(12+2)	// Size of LED display buffer - NUM_DIGITS x2 + 2 spaces for shifting

#define NUM_DIGITS			6	// Number of digits of LED display
#define SETUP_DELAY_US		5	// Serial data setup time, us
#define SHIFT_DELAY_US		5	// Serial clock hold time, us
#define PULLUP_DELAY_US		10	// Delay between disabling segment outputs and reading buttons

#define USE_EXTRA_LED_DIGIT		// Use this if you want to manage some separate LEDs dynamically
#define EXTRA_DIGIT_NUM		6	// Number of digit with separate LEDs, this value must be <= NUM_DIGITS

#define LED_DIGIT_ACT_LVL	0	// Active level for a digit (common pin for all segments)
#define LED_SEGMENT_ACT_LVL	1	// Active level for segments

//#define CLEAN_OPERATION

#define LED_SHIFT_INTERVAL	10	// in units of processWindowShifter() call period, (Tsystimer * NUM_DIGITS)

// Encoding
#define SEGA			0x01
#define SEGB			0x02
#define SEGC			0x04
#define SEGD			0x08
#define SEGE			0x10
#define SEGF			0x20
#define SEGG			0x40
#define SEGH			0x80



// Increase total count of LED digits if separate LEDs are attached
#ifdef USE_EXTRA_LED_DIGIT
#define NUM_DIGITS_TOTAL (NUM_DIGITS + 1)
#else
#define NUM_DIGITS_TOTAL NUM_DIGITS
#endif

// Inverse mask
#if LED_SEGMENT_ACT_LVL == 0
#define LED_SEGMENT_MASK 0xFF
#else
#define LED_SEGMENT_MASK 0x00
#endif

// Button inverse mask
#define RAW_BUTTONS_INVERSE_MASK 0xFF

// Shifter states
#define LED_SHIFT_DONE 	0
#define LED_SHIFT_LEFT 	1
#define LED_SHIFT_RIGHT	2


/************************************************
*			Prototypes and externs				*
************************************************/

// Internal hardware-specific functions
void led_clock_pulse(uint8_t bit);
void set_led_segments(uint8_t seg_ch);
void enable_led_segments_pullups();
void enable_led_segments();
void disable_led_segments();
void capture_button_state();


// Globals
extern uint8_t wStartPos;		// Window position. Should be read-only. 
extern uint8_t bufStartPos;		// Buffer start position. Should be read-only. 
extern uint8_t shifterState;	// Should be read-only

#ifdef USE_EXTRA_LED_DIGIT
extern volatile uint8_t extra_led_state;
#endif
extern volatile uint8_t raw_button_state;


// User functions
void initLedIndicator();
void processLedIndicator();

void setWindowStartPos(int8_t pos);
void shiftWindowPosition(int8_t increment);
void startShiftingWindowRight(void);
void startShiftingWindowLeft(void);
void setBufferStartPos(int8_t pos);
void fillLedBuffer(int8_t offset, uint8_t length, char c);
void printLedBuffer(int8_t offset, char* str);
void setComma(int8_t offset);
void clearComma(int8_t offset);



#ifdef USE_EXTRA_LED_DIGIT
void setExtraLeds(uint8_t leds_set_value);
void clearExtraLeds(uint8_t leds_clear_value);
void toggleExtraLeds(uint8_t leds_toggle_value);
#endif

#endif /* LED_INDIC_H_ */

