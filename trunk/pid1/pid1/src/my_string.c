/*
 * CFile1.c
 *
 * Created: 15.04.2013 20:09:18
 *  Author: Avega
 */ 


#include "compilers.h"
#include "my_string.h"



//-------------------------------------------------------//
// Converts uint16_t to a string
// See the i32toa_align_right() function
//-------------------------------------------------------//
void u16toa_align_right(uint16_t val, char *buffer, uint8_t len)
{
	i32toa_align_right((int32_t)val, buffer, len);
}


//-------------------------------------------------------//
//	Converts 32-bit integer to a right - aligned string
//	input:
//		val - 32-bit value to convert, signed
//		*buffer - pointer to store result data
//		len[6:0] - output string length, including \0 symbol
//				If result length < len[6:0], result will be cut to fit buffer
//				If result length > len[6:0], buffer will be filled with spaces
//		len[7] - if set, result string will be null-terminated
//-------------------------------------------------------//
void i32toa_align_right(int32_t val, char *buffer, uint8_t len)
{
	uint8_t is_negative = 0;
	if (!len)	return;
	
	if (val < 0)
	{
		val = -val;
		is_negative = 1;
	}
	
	if (len & NO_TERMINATING_ZERO)
	{
		len = len & ~NO_TERMINATING_ZERO;
		buffer += len;	
	}
	else
	{
		buffer += len;	
		*buffer = 0;
	}
	
	do
	{
		*--buffer = val % 10 + '0';
		val /= 10;
		len--;
	}
	while ((val != 0) && len);
	
	if ( (len) && (is_negative) )
	{
		*--buffer = '-';
		len--;	
	}	
	
	// Padding
	while(len--)
		*--buffer = ' ';
}




/*
// Reads bytes from FLASH into SRAM buffer until \0 is found, or
// max_length bytes have been read.
// Maximum count of bytes to read is 256 (in case max_length = 0)
void read_progmem_string(const char *src, char *dst, uint8_t max_length)
{
	char c;
	do 
	{
		c = pgm_read_byte(src++);
		*dst++ = c;
	}		
	while ((c) && (--max_length));
}
*/
