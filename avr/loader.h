// loader.h

void setup(void);
uint8_t genMenu(void);
uint8_t selectImage(uint8_t);
void loadImage(void);
uint8_t changeDir(void);
uint8_t loadHelp(void);
void dumpImage(void);

#define SEL_PROGRAM  0x00
#define SEL_FOLDER   0x01
#define SEL_HELP     0x02
#define SEL_DUMP     0x03
#define SEL_EA5      0x04  // removed

#define MODE_RAM     0x10
#define MODE_GRAM    0x20
