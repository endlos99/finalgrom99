// wire.h

void init(void);


void writeBegin(void);
void writeByte(uint8_t);
void writeSize(uint8_t);
void writeEnd(void);

void receiveBegin(void);
void receiveBytes(uint8_t *, uint8_t, uint8_t);
void receiveEnd(void);

void readBegin(void);
uint8_t readByte(void);
void readEnd(void);

void setConfig(uint8_t);

void led(uint8_t);

void flash_error(const uint8_t);
