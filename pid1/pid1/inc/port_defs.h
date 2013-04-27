/*
 * port_defs.h
 *
 * Created: 24.03.2013 12:42:38
 *  Author: Avega
 */ 


#ifndef PORT_DEFS_H_
#define PORT_DEFS_H_


// LED indicator shift register
#define PB_LED_DOUT	PB0
#define PB_LED_CLK	PB2
// LED indicator segments
#define PC_SEGA		PC0
#define PC_SEGB		PC1
#define PC_SEGC		PC2
#define PC_SEGD		PC3
#define PC_SEGE		PC4
#define PB_SEGF		PB3
#define PB_SEGG		PB4
#define PB_SEGH		PB5




// Port D 
#define PD_RXD		PD0
#define PD_TXD		PD1
#define PD_M1		PD2
#define PD_M2		PD3
#define PD_HEATER	PD4
#define PD_HEAT_INDIC	PD5
#define PD_SYNCA	PD6
#define PD_SYNCB	PD7	

// Port C
#define PC_TSENCE	PC5


// Port B
#define PB_BEEPER	PB1



#endif /* PORT_DEFS_H_ */