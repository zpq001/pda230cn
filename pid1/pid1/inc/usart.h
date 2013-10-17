/*
 * usart.h
 *
 * Created: 31.03.2013 23:05:10
 *  Author: Avega
 */ 


#ifndef USART_H_
#define USART_H_

// Functions for log optimization
void logU16p(uint16_t val);
void logI32p(int32_t val);

void USART_send( uint8_t data );
void USART_sendstr(char* str);





#endif /* USART_H_ */