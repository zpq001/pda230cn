/*
 * avr_port_macros.h
 *
 * Created: 27.03.2013 20:24:00
 *  Author: Avega
 */ 


#ifndef AVR_PORT_MACROS_H_
#define AVR_PORT_MACROS_H_


#define PM_SETOUT(port,bit)		(DDR##port |= (1<<(bit)))
#define PM_SETIN(port,bit)		(DDR##port &= ~(1<<(bit)))
#define PM_SETPULLUP(port,bit)	(PORT##port |= (1<<(bit)))
#define PM_SETHIZ(port,bit)		(PORT##port &= ~(1<<(bit)))
#define PM_SETL(port,bit)		(PORT##port &= ~(1<<(bit)))
#define PM_SETH(port,bit)		(PORT##port |= (1<<(bit)))
#define PM_PINH(port,bit)		(PIN##port & (1<<(bit)))
#define PM_PINL(port,bit)		(!PM_PINH(port,bit))

#define AAA(port,bit)			(PORT##port |= (1<<(bit)))

#endif /* AVR_PORT_MACROS_H_ */