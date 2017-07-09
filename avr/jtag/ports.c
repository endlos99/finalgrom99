/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/*                                                     */
/* Adapted for FinalGROM 99 by Ralph Benzinger         */
/*******************************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "../lib/pff.h"  // use main pff
#include "../wire.h"

#include "ports.h"


#define pTMS (1 << PB0)
#define pTDO (1 << PB6)
#define pTDI (1 << PB7)
#define pTCK (1 << PC5)


// SD card handling
static UINT buf_avail;
static uint16_t buf_idx;
extern uint8_t buffer[512];
static uint32_t buf_len;
void flash_error(const uint8_t count);


// config ports
void initJTAGPorts()
{
  DDRB |= 0x81;  // activate JTAG pins
  DDRC |= 0x20;
  buf_len = buf_avail = buf_idx = 0;
}


/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void setPort(short p, short val)
{
  if (val) {
    switch (p) {
      case TCK:  PORTC |= pTCK;  break;
      case TMS:  PORTB |= pTMS;  break;
      case TDI:  PORTB |= pTDI;  break;
    }
  } else {
    switch (p) {
      case TCK:  PORTC &= ~pTCK;  break;
      case TMS:  PORTB &= ~pTMS;  break;
      case TDI:  PORTB &= ~pTDI;  break;
    }
  }
}


/* toggle tck LH.  No need to modify this code.  It is output via setPort. */
void pulseClock()
{
    setPort(TCK,0);  /* set the TCK port to low  */
    setPort(TCK,1);  /* set the TCK port to high */
}


/* readByteJ:  Implement to source the next byte from your XSVF file location */
/* read in a byte of data from the prom */
void readByteJ(unsigned char *data)
{
  if (buf_idx >= buf_avail) {
    if (pf_read(buffer, sizeof(buffer), &buf_avail))
      flash_error(2);
    if (buf_avail == 0)
      flash_error(2);  // moved beyond EOF
    buf_len += buf_avail;
    buf_idx = 0;
  }
  *data = buffer[buf_idx++];
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
unsigned char readTDOBit()
{
    /* You must return the current value of the JTAG TDO signal. */
    return PINB & pTDO ? 1 : 0;
}

/* waitTime:  Implement as follows: */
/* REQUIRED:  This function must consume/wait at least the specified number  */
/*            of microsec, interpreting microsec as a number of microseconds.*/
/* REQUIRED FOR SPARTAN/VIRTEX FPGAs and indirect flash programming:         */
/*            This function must pulse TCK for at least microsec times,      */
/*            interpreting microsec as an integer value.                     */
/* RECOMMENDED IMPLEMENTATION:  Pulse TCK at least microsec times AND        */
/*                              continue pulsing TCK until the microsec wait */
/*                              requirement is also satisfied.               */
void waitTime(long microsec)
{
#if 0
    static long tckCyclesPerMicrosec    = 4; /* must be at least 1 */
    long        tckCycles   = microsec * tckCyclesPerMicrosec;
    long        i;

    /* This implementation is highly recommended!!! */
    /* This implementation requires you to tune the tckCyclesPerMicrosec 
       variable (above) to match the performance of your embedded system
       in order to satisfy the microsec wait time requirement. */
    for ( i = 0; i < tckCycles; ++i )
    {
      pulseClock();  // 250 ns
    }
#endif
    
#if 0
    /* Alternate implementation */
    /* For systems with TCK rates << 1 MHz;  Consider this implementation. */
    /* This implementation does not work with Spartan-3AN or indirect flash
       programming. */
    if ( microsec >= 50L )
    {
        /* Make sure TCK is low during wait for XC18V00/XCFxxS */
        /* Or, a running TCK implementation as shown above is an OK alternate */
        setPort( TCK, 0 );

        /* Use Windows Sleep().  Round up to the nearest millisec */
        _sleep( ( microsec + 999L ) / 1000L );
    }
    else    /* Satisfy FPGA JTAG configuration, startup TCK cycles */
    {
        for ( i = 0; i < microsec;  ++i )
        {
            pulseClock();
        }
    }
#endif

#if 1    
    /* Alternate implementation */
    /* This implementation is valid for only XC9500/XL/XV, CoolRunner/II CPLDs, 
       XC18V00 PROMs, or Platform Flash XCFxxS/XCFxxP PROMs.  
       This implementation does not work with FPGAs JTAG configuration. */
    /* Make sure TCK is low during wait for XC18V00/XCFxxS PROMs */
    /* Or, a running TCK implementation as shown above is an OK alternate */
    setPort( TCK, 0 );
    /* Round up to the nearest millisec */
    long ms = microsec / 1000L + 1L;
    while (ms--) {
      _delay_ms(1);
    }
#endif
}
