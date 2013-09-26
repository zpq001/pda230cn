/*
 * progmem_func.c
 *
 * Created: 26.09.2013 20:56:21
 *  Author: Avega
 */ 

#include "compilers.h"
#include "progmem_func.h"

// Reads specified number of bytes from FLASH memory into SRAM
void PGM_read_block(void *dst, const void *src, uint8_t count)
{
	while(count--)
	{
		*(uint8_t*)dst++ = pgm_read_byte((const PROGMEM uint8_t*)src++);
	}
}

