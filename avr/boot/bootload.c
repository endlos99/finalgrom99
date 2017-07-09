// bootloader for µC update


/*
 * BOOTSZ0 = BOOTSZ1 = 0 for max size (2K for 168, 4K for 328) [default]
 * BOOTRST = 1           for start at application [default]
 *
 * boot loader address: 3800 (word) = 7000 (byte)
 */

/*
 * We could start in the bootloader, and then jump to the FinalGROM
 * if we don't find an update file.
 * This wil delay the startup sequence by 1 second or more, though.
 */


#include <avr/io.h>
#include <avr/boot.h>
#include <util/delay.h>

#include <lib/pff.h>


#define PAGE_SIZE_B SPM_PAGESIZE

//static void (*start)(void) = (void *)0x0000;
//static void (*bootloader)(void) = (void *)0x3800;  // word address for m328


// SD card data structures
static FATFS sd_fs;
static uint8_t buffer[PAGE_SIZE_B];

static void init(void);
static void fill_buffer(uint8_t *data, uint32_t count);
static void write_page(uint32_t addr);
static void led(uint8_t on);
static void flash_error(const uint8_t count);


// invoked by FinalGROM menu generator
int main(void)
{
  UINT bytes_read;

  init();
  led(1);
  
  if (pf_mount(&sd_fs))
    flash_error(1);
  
  if (pf_open("UPDATE.AVR"))
    flash_error(2);
    //(*start)();  // start FinalGROM

  // prepare for update
  uint32_t addr = 0;
  eeprom_busy_wait();

  // update program memory page by page
  while (1) {
    if (pf_read(buffer, PAGE_SIZE_B, &bytes_read))
      flash_error(2);
    if (bytes_read == 0)
      break;
    
    fill_buffer(buffer, (uint32_t)bytes_read);
    write_page(addr);
    
    addr += PAGE_SIZE_B;
  }
  
  // wrap up
  boot_rww_enable();
  led(0);

  // cannot restart, thus endless loop
  flash_error(0);
  return 0;
}


// prepare page buffer for write
static void fill_buffer(uint8_t *data, uint32_t count)
{
  uint8_t *p = data;
  
  for (uint32_t i = 0; i < count; i += 2) {
    uint16_t w = (uint16_t)*p++;
    w += (uint16_t)*p++ << 8;
    boot_page_fill(i, w);
  }
}


// write a single page to Flash
static void write_page(uint32_t addr)
{
    boot_page_erase(addr);
    boot_spm_busy_wait();

    boot_page_write(addr);
    boot_spm_busy_wait();
}


// setup wires
static void init()
{
  DDRB = 0x02;  // need only LED
  DDRC = 0x00;
  DDRD = 0x00;
}


// turn busy indicator on or off
static void led(uint8_t on)
{
  if (on) {
    PORTB |= 1 << PB1;
  } else {
    PORTB &= ~(1 << PB1);
  }
}


// success or fail: blink LED
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
