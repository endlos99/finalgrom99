// mock types for avr

// stdint
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

// pgmspace.h

#define PROGMEM
#define _delay_ms(x)
#define _delay_us(x)
uint8_t pgm_read_byte(const uint8_t *p);
