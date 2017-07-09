/*
 * wire.c: low-level signal handling
 */


#include <avr/io.h>
#include <util/delay.h>

#include "wire.h"


/*
 * pins
 *
 * B7  TDI                     D7  CD7
 * B6  TDO       C6  RESET     D6  CD6
 * B5  SDC       C5  TCK       D5  CD5
 * B4  SDI (MI)  C4  CCMD0(CY) D4  CD4
 * B3  SDO (MO)  C3  CX        D3  CD3
 * B2  SDS       C2  CCLK      D2  CD2
 * B1  BUSY      C1  CCMD2     D1  CD1
 * B0  TMS       C0  CCMD1     D0  CD0
 */

#define pTDI    (1 << PB7)
#define pTDO    (1 << PB6)
#define pSDC    (1 << PB5)
#define pSDI    (1 << PB4)
#define pSDO    (1 << PB3)
#define pSDS    (1 << PB2)
#define pBUSY   (1 << PB1)
#define pTMS    (1 << PB0)

#define pRESET  (1 << PC6)
#define pTCK    (1 << PC5)
#define pCCMD0  (1 << PC4)
#define pCX     (1 << PC3)
#define pCCLK   (1 << PC2)
#define pCCMD2  (1 << PC1)
#define pCCMD1  (1 << PC0)
#define pC      0x1f

#define sRUN    (                           pCCLK)        // 000
#define sSIZE   (pCCMD2                   | pCCLK | pCX)  // 100
#define sCONF   (pCCMD2 |          pCCMD0 | pCCLK | pCX)  // 101
#define sDUMP   (pCCMD2 | pCCMD1          | pCCLK | pCX)  // 110
#define sLOAD   (pCCMD2 | pCCMD1 | pCCMD0 | pCCLK | pCX)  // 111

static uint8_t listen_sequence[] = {
  0x99, 'O', 'K', 'F', 'G', '9', '9', 0x99, 0x00
};


/*
 * init pins
 */

void init()
{
  // SPI and JTAG, busy indicator
  DDRB = pBUSY;  // keep pins inactive
  PORTB = 0x00;
   // control lines
  DDRC = pC;
  PORTC = sLOAD;  // LOAD/RLOAD
  // data µC <-> CPLD
  DDRD = 0xff;
  PORTD = 0x00;
}


/*
 * transfer RAM data to CPLD
 */

// write byte stream
void writeBegin()
{
  DDRC |= pCX;  // CX used for RWE
  PORTC = sLOAD;  // LOAD/RLOAD
  _delay_us(7);  // time for CPLD to pick up value (3+ clocks)
  DDRD = 0xff;  // CDX sends
  // cart offline now
}


// write single byte
void writeByte(uint8_t b)
{
  // CCLK high
  PORTD = b;
  PORTC &= ~pCCLK;  // CCLK low
  PORTC &= ~pCX;  // RWE low
  // read CDX data
  PORTC |= pCX;  // RWE high
  PORTC |= pCCLK;  // CCLK high
}


// sets start addr and sizing information
//   size = 1 r - X X X X X = GROM mask (r=0 reset)
//          0 X X X X X X X = bank mask
void writeSize(uint8_t size) //OK
{
  PORTD = size;
  PORTC = sSIZE;  // set size
  _delay_us(7);
  PORTC &= ~pCCLK;  // CCLK low
  PORTC |= pCCLK;  // CCLK high -> read size
  _delay_us(0.2);
  PORTC = sLOAD;  // clear set size
  _delay_us(7);
}


// cartridge configurations
void setConfig(uint8_t config)
{
  PORTC = sCONF;
  PORTD = config;
  _delay_us(7);
  PORTC = sLOAD;
  _delay_us(7);
}


void writeEnd()
{
}


/*
 * read TI data via serial connection
 * - clock: CLR @>6000
 * - send:  CLR @>7xyz, where xyz = 2*byte to send
 */

void receiveBegin()
{
  DDRC &= ~pCX;  // CX used for SCLK
  PORTD = 0x00;  // CDX receives
  DDRD = 0x00;
  PORTC = sRUN;  // RUN/-
  _delay_us(7);
  // cart online again
}


void receiveBytes(uint8_t *buffer, uint8_t length, uint8_t needs_seq)
{
  // get to known state (IMGEN low), initial CLR @>6000 comes too late
  while (PINC & pCX);  // read CX/SCLK

  // look for listen sequence if needed
  while (needs_seq) {
    uint8_t *p = listen_sequence;
    while (*p) {
      while (!(PINC & pCX));  // wait for high clock
      uint8_t b = PIND;  // read byte
      while (PINC & pCX);  // wait for low clock
      if (b != *p)
        break;  // redo sequence
      ++p;
    }
    if (!*p)
      needs_seq = 0;  // received complete sequence
  }

  // read bytes from TI
  for (uint8_t i = length; i > 0; --i) {
    // wait for clock bit (SCLK)
    while (!(PINC & pCX));
    // read byte
    *buffer++ = PIND;
    // wait until SCLK low again
    while (PINC & pCX);
  }
}


void receiveEnd()
{
  PORTC = sLOAD;  // cart offline
  _delay_us(7);
}


/*
 * transfer RAM data to uC/SD
 */

void readBegin()
{
  PORTD = 0x00;  // CDX receives
  DDRD = 0x00;
  PORTC = sDUMP;  // LOAD/RDUMP
  _delay_us(7);
  // cart offline for dump
}


uint8_t readByte()
{
  uint8_t b = PIND;
  PORTC &= ~pCCLK;  // next byte
  PORTC |= pCCLK;
  return b;
}


void readEnd()
{
  // followed by receiveBegin()
}


/*
 * activity indicator
 */

void led(uint8_t on)
{
  if (on) {
    PORTB |= pBUSY;
  } else {
    PORTB &= ~pBUSY;
  }
}


/*
 * fatal error: blink LED
 */

void flash_error(const uint8_t count)
{
  while(1) {
    for (uint8_t i = 0; i < count; ++i) {
      led(1);
      _delay_ms(250);
      led(0);
      _delay_ms(250);
    }
    _delay_ms(600);
  }
}
