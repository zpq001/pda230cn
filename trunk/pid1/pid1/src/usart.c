/*
 * usart.c
 *
 * Created: 31.03.2013 23:05:01
 *  Author: Avega
 */ 



#include "compilers.h"
#include "port_defs.h"
#include "usart.h"
#include "my_string.h"


static char str[20];


//-------------------------------------------------------//
// Sends a char over UART
// Blocks until transmit is complete
//-------------------------------------------------------//
void USART_send( uint8_t data )
{
	//  Wait for empty transmit buffer 
	while ( !( UCSRA & (1<<UDRE)) );
	//  Put data into buffer, sends the data
	UDR = data;
}

//-------------------------------------------------------//
// Sends a null-terminated char string over UART
// null-terminating symbol is not transmitted
// Blocks until transmit is complete
//-------------------------------------------------------//
void USART_sendstr(char* str)
{
	while (*str)
	{
		USART_send(*str++);
	}
}

//-------------------------------------------------------//
// Function to log uint16_t type data
//-------------------------------------------------------//
void logU16p(uint16_t val)
{
	u16toa_align_right(val,str,6);
	USART_sendstr(str);
}

//-------------------------------------------------------//
// Function to log int32_t type data
//-------------------------------------------------------//
void logI32p(int32_t val)
{
	i32toa_align_right(val,str,12);
	USART_sendstr(str);
}



