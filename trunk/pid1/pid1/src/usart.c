/*
 * usart.c
 *
 * Created: 31.03.2013 23:05:01
 *  Author: Avega
 */ 



#include "compilers.h"
#include "port_defs.h"
#include "usart.h"







void USART_send( uint8_t data )
{
	UCSRA |= (1<<TXC);                 // Clear flag
	UDR = data;
	while ( !(UCSRA & (1<<TXC)) );  // Wait
}


void USART_sendstr(char* str)
{
	uint8_t i = 0;
	while (str[i])
	{
		USART_send(str[i++]);
	}
}





