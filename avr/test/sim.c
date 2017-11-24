/*
 * FinalGROM 99 simulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "mocktypes.h"
#include <loader.h>
#include <wire.h>


char m_dirname[256];
char m_send[20];
extern uint8_t *path;
//extern uint8_t selection[];


/*
 * mocked hardware
 */

FILE *m_from = NULL;
FILE *m_fgrom = NULL;
FILE *m_fdump = NULL;
FILE *m_f = NULL;
char m_buff[10000];


/*
 * mocked avr library
 */

uint8_t pgm_read_byte(const uint8_t *p)
{
  return *p;
}


/*
 * mocked wire.c funktions
 */

void init()
{
  m_fdump = fopen("DUMP.bin", "wb");
}

void writeBegin()
{
  puts("WRITE BEGIN");
  m_from = fopen("ROM.sim", "wb");
  m_fgrom = fopen("GROM.sim", "wb");
}

void writeByte(uint8_t b)
{
  fputc((int) b, m_f);
}

void writeSize(uint8_t b)
{
  if (b & 0x80) {
    printf("SET GROM = >%02x\n", b);
    m_f = m_fgrom;  // write to GROM
  } else {
    printf("SET ROM = >%02x\n", b);
    m_f = m_from;  // write to ROM
  }
}

void writeEnd()
{
  puts("WRITE END");
  fclose(m_from);
  fclose(m_fgrom);
}


void receiveBegin()
{
}

void receiveBytes(uint8_t *buffer, uint8_t length, uint8_t reload)
{
  memcpy((char*)buffer, m_send, length);
  printf("selected: %.8s\n", buffer);
}

void receiveEnd()
{
}


void readBegin()
{
}

uint8_t readByte()
{
  return 0;
}

void readEnd()
{
}


void setConfig(uint8_t conf)
{
  printf("configuration = >%02x\n", conf);
}


void led(uint8_t on)
{
}

void flash_error(const uint8_t count)
{
  printf("flash error: %d\n", count);
  exit(1);
}

void sig(const uint8_t signal)
{
  printf("signal: %d\n", signal);
}


/*
 * mocked pff library
 */

#define AM_DIR	0x10	/* Directory */

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef unsigned int	UINT;

typedef struct {
  	DWORD	fsize;		/* File size */
	DWORD   clust;
	BYTE	fattrib;	/* Attribute */
	char	fname[13];	/* File name */
} FILINFO;

typedef int FRESULT;


DIR *m_dir = 0;
FILE *m_file = 0;
struct stat m_stat;

FRESULT pf_mount(void* fs)
{
  return 0;
}

FRESULT pf_open(const char* path)
{
  char buf[256];
  strcpy(buf, m_dirname);
  strcat(buf, path);
  printf("fopen: %s\n", buf);
  m_file = fopen(buf, "rb");
  return !m_file;
}

FRESULT pf_fnoopen(const FILINFO* fno)
{
  char buf[256];
  strcpy(buf, m_dirname);
  strcat(buf, fno->fname);
  printf("fnoopen: %s\n", buf);
  m_file = fopen(buf, "rb");
  return !m_file;
}

FRESULT pf_read(void *buff, UINT btr, UINT* br)
{
  printf("fread: %d\n", btr);
  size_t n = fread(buff ? buff : m_buff, 1, btr, m_file);
  if (!buff)
    fwrite(m_buff, 1, n, m_f);
  *br = n;
  return 0;
}

FRESULT pf_lseek (DWORD ofs)
{
  printf("lseek: %ld\n", ofs);
  fseek(m_file, ofs, SEEK_SET);
  return 0;
}

FRESULT pf_opendir(DIR* dj, const char* path)
{
  printf("opendir: %s\n", m_dirname);
  m_dir = opendir(m_dirname);
  if (!m_dir) {
    printf("error opening %s\n", m_dirname);
    return 1;
  }
  return 0;
}

FRESULT pf_readdir(DIR* dj, FILINFO* fno)
{
  struct dirent *e;
  char buf[256];

  while (1) {
    e = readdir(m_dir);
    if (!e) {
      printf("end of dir\n");
      fno->fname[0] = 0;
      return 0;
    }
    if (e->d_name[0] != '.')
      break;
  }

  strcpy(buf, m_dirname);
  strcat(buf, e->d_name);
  stat(buf, &m_stat);

  strcpy(fno->fname, e->d_name);
  fno->fsize = m_stat.st_size;
  fno->fattrib = (m_stat.st_mode & S_IFDIR) ? AM_DIR : 0;
  if (fno->fattrib)
    printf("found dir: %s\n", fno->fname);
  else
    printf("found file: %s (%ld)\n", fno->fname, fno->fsize);
  return 0;
}

FRESULT pf_write (const void* buff, UINT btw, UINT* bw)
{
  printf("fwrite: %d\n", btw);
  *bw = fwrite(buff, 1, btw, m_fdump);
  return 0;
}

int xsvfExecute(void)
{
  return 0;
}

void initJTAGPorts(void)
{
}

void rcode(uint8_t n)
{
  printf("return code = %d\n", n);
}


/*
 * main program
 */

int main(int argc, char **argv)
{
  // setup test
  if (argc < 2) {
    printf("usage: %s <dir/> [<name>]\n", argv[0]);
    exit(1);
  }
  strcpy(m_dirname, argv[1]);

  // setup cart
  init();

  // create menu loader
  int imm = genMenu();
  if (imm) {
    puts("<IMMEDIATE LOAD>");
    loadImage();
  }

  if (argc > 2) {
    // wait for menu selection
    char *p = m_send;  // will be received

    // filename (ASCII)
    char *q = argv[2];
    for (uint8_t i = 0; i < 8; ++i)
      *p++ = *q ? *q++ : 0;

    // type information (hex)
    q = argv[3];
    char hex[3] = "00";
    for (uint8_t i = 0; i < 8; i += 2) {
      hex[0] = *q++;  hex[1] = *q++;
      long v = strtol(hex, NULL, 16);
      *p++ = (char)v;
    }
    printf("loading: %s\n", m_send);

    // execute selection action
    uint8_t s = selectImage(0);
    if (s == SEL_PROGRAM) {
      printf("load image ...\n");
      loadImage();
    } else if (s == SEL_HELP) {
      printf("generating help ...\n");
      loadHelp();  // show help file
    } else if (s == SEL_DUMP) {
      printf("loading and dumping image ...\n");
      loadImage();
      dumpImage();
    } else {
      changeDir();  // cd into selected folder
      printf("-> cd %s/%.8s\n", argv[1], m_send);
    }
  }

}
