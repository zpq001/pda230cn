/*
 * my_string.h
 *
 * Created: 15.04.2013 20:10:02
 *  Author: Avega
 */ 


#ifndef MY_STRING_H_
#define MY_STRING_H_

#define NO_TERMINATING_ZERO	0x80

void u16toa_align_right(uint16_t val, char *buffer, uint8_t len);
void i32toa_align_right(int32_t val, char *buffer, uint8_t len);

//void read_progmem_string(const char *src, char *dst, uint8_t max_length);

#endif /* MY_STRING_H_ */