/*
 * led_indic.c
 *
 * Created: 24.03.2013 18:23:11
 *  Author: Avega
 */ 


#include "compilers.h"
#include "systimer.h"
#include "led_indic.h"
#include "soft_timer.h"

// User
uint8_t led_data_buffer[LED_BUFFER_LENGTH];			
uint8_t wStartPos;							// Number of first buffer item displayed on the LED display (window start)
uint8_t bufStartPos;						// Number of buffer start position for writing
#ifdef 	USE_EXTRA_LED_DIGIT
	volatile uint8_t extra_led_state;
#endif
	// Global button state
volatile uint8_t raw_button_state;


// Internal
static uint8_t bufActivePos;				// number of buffer item displayed to active LED digit
static uint8_t wActivePos;					// number of active LED display digit
uint8_t shifterState;	
static SoftTimer8b_t shiftTimer;			// used for LED window shift



/**********************************************************
*					Internal functions					  *
**********************************************************/

//---------------------------------------------//
// Decodes normal string literal to the 
//	7-segment representation
//---------------------------------------------//
static uint8_t decode_led_char(char c)
{
	switch(c)
	{
		case 'O':
		case '0': return (SEGA | SEGB | SEGC | SEGD | SEGE | SEGF);
		case '1': return (SEGB | SEGC );
		case '2': return (SEGA | SEGB | SEGD | SEGE | SEGG);
		case '3': return (SEGA | SEGB | SEGC | SEGD | SEGG);
		case '4': return (SEGB | SEGC | SEGF | SEGG);
		case 'S':
		case '5': return (SEGA | SEGC | SEGD | SEGF | SEGG);
		case '6': return (SEGA | SEGC | SEGD | SEGE | SEGF | SEGG);
		case '7': return (SEGA | SEGB | SEGC );
		case '8': return (SEGA | SEGB | SEGC | SEGD | SEGE | SEGF | SEGG);
		case '9': return (SEGA | SEGB | SEGC | SEGD | SEGF | SEGG);
		case '.': return (SEGH);
		case ',': return (SEGH);
		case '-': return (SEGG);
		case '_': return (SEGD);
		case ' ': return 0;
		case 0xB0:	return (SEGA | SEGB | SEGF | SEGG);		// Degree sin
		case 'C':	return (SEGA | SEGD | SEGE | SEGF);
		case 'F':	return (SEGA | SEGE | SEGF | SEGG);
		case 'N':	return (SEGC | SEGE | SEGG);
		case 'D': return (SEGB | SEGC | SEGD | SEGE | SEGG);
		case 'P': return (SEGA | SEGB | SEGE | SEGF | SEGG);
		case 'E': return (SEGA | SEGD | SEGE | SEGF | SEGG);
		default:  return c;	
	}
}

//---------------------------------------------//
// Increments index. If index falls out of buffer range,
// index is wrapped
//---------------------------------------------//
static inline uint8_t inc_buffer_position(uint8_t position)
{
	return (position >= LED_BUFFER_LENGTH - 1) ? 0 : position + 1;
}

//---------------------------------------------//
// Decrements index. If index falls out of buffer range,
// index is wrapped
//---------------------------------------------//
static inline uint8_t dec_buffer_position(uint8_t position)
{
	return (position == 0) ? LED_BUFFER_LENGTH - 1 : position - 1;
}

//---------------------------------------------//
// Ensures that index is inside the buffer
// -128 <= index <= 127
//---------------------------------------------//
static uint8_t wrap_led_buffer_index(int8_t buf_index)
{
	uint8_t new_index_u;
	new_index_u = (uint8_t) ((buf_index < 0) ? -buf_index : buf_index);
	if (new_index_u >= LED_BUFFER_LENGTH)
		new_index_u = new_index_u % LED_BUFFER_LENGTH;
	new_index_u = ((buf_index >= 0) || (new_index_u == 0)) ? new_index_u :  LED_BUFFER_LENGTH - new_index_u ;
	return new_index_u;
}

//---------------------------------------------//
// Performs window shifting
//---------------------------------------------//
static void processWindowShifter(void)
{
	if (shifterState == LED_SHIFT_DONE)
		return;
	
	// Process shift timer
	processSoftTimer8b(&shiftTimer);	
	
	if (shiftTimer.FTop)
	{
		// Check if finished
		if (wStartPos == bufStartPos)	
		{
			// Update state
			shifterState = LED_SHIFT_DONE;
		}
		else if (shifterState == LED_SHIFT_RIGHT)
		{
			wStartPos = inc_buffer_position(wStartPos);	
		}
		else
		{
			wStartPos = dec_buffer_position(wStartPos);		
		}
	}

}


/**********************************************************
*						User functions					  *
**********************************************************/


//---------------------------------------------//
//	Initialization
//	Call this function at start of your program
//---------------------------------------------//
void initLedIndicator()
{
	uint8_t i;
	// First disable outputs
	disable_led_segments();
	// Fill shift register with non-active levels
	for (i=0;i<NUM_DIGITS_TOTAL;i++)
	{
		led_clock_pulse(!LED_DIGIT_ACT_LVL);
	}
	// Now enable outputs
	enable_led_segments();
	
	// Reset pointers
	wStartPos = 0;
	bufStartPos = 0;
	bufActivePos = 0;
	wActivePos = 0;
	#ifdef 	USE_EXTRA_LED_DIGIT
	extra_led_state = 0;
	#endif
	
	// Clear the buffer 
	fillLedBuffer(0,LED_BUFFER_LENGTH,' ');
	
	// Initialize shift timer - always enabled, but 
	//	processed only when shifting is active
	shifterState = LED_SHIFT_DONE;
	shiftTimer.Enabled = 1;
	shiftTimer.RunOnce = 0;
	shiftTimer.Top = LED_SHIFT_INTERVAL-1;
}

//---------------------------------------------//
//	Main LED indicator function
// Dynamically outputs data from buffer to the LEDs
// Call this function N times per sec to get N/NUM_DIGITS_TOTAL display update rate, where
// NUM_DIGITS_TOTAL is number_of_digits_of_LED_display if no extra LEDs are used or
// (number_of_digits_of_LED_display + 1) otherwise.
//---------------------------------------------//
void processLedIndicator()
{
	uint8_t next_wActivePos;
	
	// Turn off segments
	#ifdef CLEAN_OPERATION
	disable_led_segments();
	#endif
	
	
	// Perform regular shift
	led_clock_pulse(!LED_DIGIT_ACT_LVL);
	
	switch (wActivePos)
	{
	case 0:
		// Get buttons
		#ifndef CLEAN_OPERATION
		disable_led_segments();
		#endif
		enable_led_segments_pullups();
		_delay_us(PULLUP_DELAY_US);
		capture_button_state();
		#ifndef CLEAN_OPERATION
		enable_led_segments();
		#endif
		// Start new cycle with active digit level
		led_clock_pulse(LED_DIGIT_ACT_LVL);
		// Always start new cycle from specified buffer start position
		bufActivePos = wStartPos;
		next_wActivePos = wActivePos + 1;
		break;
	case NUM_DIGITS_TOTAL-1:
		next_wActivePos = 0;
		// Shift LED window
		processWindowShifter();	
		break;
	default:
		next_wActivePos = wActivePos + 1;
		break;
	}		
	
	#ifdef 	USE_EXTRA_LED_DIGIT
	if (wActivePos == EXTRA_DIGIT_NUM)
	{
		// Output separate LEDs state
		set_led_segments(extra_led_state);
	}
	else
	#endif
	{
		// Output new segment data
		set_led_segments(led_data_buffer[bufActivePos]);
		// Wrap around buffer length
		bufActivePos = inc_buffer_position(bufActivePos );
	}
		
	wActivePos = next_wActivePos;
	
	#ifdef CLEAN_OPERATION
	enable_led_segments();
	#endif

}


//-------------------------------------------------------//
//-------------------------------------------------------//



//---------------------------------------------//
// Sets displayed window start position
//---------------------------------------------//
void setWindowStartPos(int8_t pos)
{
	wStartPos = wrap_led_buffer_index(pos);
}

//---------------------------------------------//
// Shift displayed window start position
//---------------------------------------------//
void shiftWindowPosition(int8_t increment)
{
	wStartPos = wrap_led_buffer_index((int8_t)wStartPos + increment);
}

//---------------------------------------------//
// Modifies bufStartPos and starts shifting 
//	LED window right 
//---------------------------------------------//
void startShiftingWindowRight(void)
{
	// Clear spaces between menu states
	fillLedBuffer(NUM_DIGITS,2,' ');
	// Set new buffer position for writing
	bufStartPos = wrap_led_buffer_index( (int8_t)bufStartPos + NUM_DIGITS + 2 );
	// Set shifter state
	shifterState = LED_SHIFT_RIGHT;
}

//---------------------------------------------//
// Modifies bufStartPos and starts shifting 
//	LED window left 
//---------------------------------------------//
void startShiftingWindowLeft(void)
{
	// Clear spaces between menu states
	fillLedBuffer(-2,2,' ');
	// Set new buffer position for writing
	bufStartPos = wrap_led_buffer_index( (int8_t)bufStartPos - (NUM_DIGITS + 2) );
	// Set shifter state
	shifterState = LED_SHIFT_LEFT;
}

//---------------------------------------------//
// Sets buffer start position
//---------------------------------------------//
void setBufferStartPos(int8_t pos)
{
	bufStartPos = wrap_led_buffer_index(pos);
}

//---------------------------------------------//
// Fills buffer from (bufStartPos + offset) 
//	position with single char
//---------------------------------------------//
void fillLedBuffer(int8_t offset, uint8_t length, char c)
{
	uint8_t val = decode_led_char(c);
	// Ensure index is inside the buffer
	uint8_t position = wrap_led_buffer_index((int8_t)bufStartPos + offset);
	while(length--)
	{
		led_data_buffer[position] = val;
		position = inc_buffer_position(position);
	}
}

//---------------------------------------------//
// Prints a string to buffer in circular manner.
// Arguments:
//		offset	- buffer index to start with (with respect to bufStartPos)
//		str - null-terminated char string
// Comma and dot (".", ",") get special processing - first occurrence is added to
// a preceding char, next are displayed as a single digit
//---------------------------------------------//
void printLedBuffer(int8_t offset, char* str)
{
	char c;
	uint8_t decoded_sym;
	uint8_t state = 0x02;
	uint8_t comma_pos;
	// Ensure index is inside the buffer
	uint8_t buffer_position = wrap_led_buffer_index((int8_t)bufStartPos + offset);
	comma_pos = buffer_position;
	// Output chars
	while((c = *str++))
	{
		decoded_sym = decode_led_char(c);
		state = (decoded_sym == SEGH) ? (state | 0x01) : (state & ~0x01);
		if (state == 0x01)			
		{
			led_data_buffer[comma_pos] |= decoded_sym;
		}
		else
		{
			led_data_buffer[buffer_position] = decoded_sym;	
		}	
		comma_pos = buffer_position;
		if (state != 0x01)
		{
			buffer_position = inc_buffer_position(buffer_position);
		}
		state = (state & 0x01) ? 0x03 : 0x00;		
	}
}


//---------------------------------------------//
// Sets comma sign in the (bufStartPos + offset) 
//	buffer cell
//---------------------------------------------//
void setComma(int8_t offset)
{
	// Ensure index is inside the buffer
	uint8_t position = wrap_led_buffer_index((int8_t)bufStartPos + offset);
	led_data_buffer[position] |= (SEGH);
}

//---------------------------------------------//
// Removes comma sign in the (bufStartPos + offset) 
//	buffer cell
//---------------------------------------------//
void clearComma(int8_t offset)
{
	// Ensure index is inside the buffer
	uint8_t position = wrap_led_buffer_index((int8_t)bufStartPos + offset);
	led_data_buffer[position] &= ~(SEGH);
}



//-------------------------------------------------------//
//-------------------------------------------------------//



// Functions for atomic access to the additional external LEDs
// 	In order to ensure atomic operations, disabling global interrupts might be required, while
// 	updating LEDs

#ifdef USE_EXTRA_LED_DIGIT

//---------------------------------------------//
// Function to set LEDs
// Argument is ORed with current LED state
//---------------------------------------------//
inline void setExtraLeds(uint8_t leds_set_value)
{
		//asm("cli");
		extra_led_state |= leds_set_value;
		//asm("sei");
}

//---------------------------------------------//
// Function to clear LEDs
// Argument is AND NOTed with current LED state
//---------------------------------------------//
inline void clearExtraLeds(uint8_t leds_clear_value)
{
		//asm("cli");
		extra_led_state &= ~leds_clear_value;
		//asm("sei");
}

//---------------------------------------------//
// Function to toggle LEDs
// Argument is XORed with current LED state
//---------------------------------------------//
inline void toggleExtraLeds(uint8_t leds_toggle_value)
{
		//asm("cli");
		extra_led_state ^= leds_toggle_value;
		//asm("sei");
}


#endif





