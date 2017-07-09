# -*- makefile -*-
#
# Makefile for Simulated FinalGROM 99
#
# Uses gcc instead of avr-gcc to build an executable for x64/x86


OPTIONS =

# Target file name (without extension).
TARGET = sim

# List C source files here. (C dependencies are automatically generated.)
SRC = $(TARGET).c loader.c

# Compiler flag to set the C Standard level.
CSTANDARD = -std=gnu99

# Place -D or -U options here
CDEFS = -D SIMULATE $(OPTIONS)

# Place -I options here
CINCS = -I ..

# Compiler flags.
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += $(CSTANDARD)

# ---------------------------------------------------------------------------

# Define programs and commands.
CC = gcc
COPY = cp
REMOVE = rm -f

# Define all object files.
OBJ = $(SRC:.c=.o)

# Combine all necessary flags and optional flags.
ALL_CFLAGS = -I. $(CFLAGS)

# ---------------------------------------------------------------------------

# Default target.
all: clean prep build

prep:
	$(COPY) ../loader.c loader.c

build: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

%.o : %.c
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 

clean:
	$(REMOVE) loader.c
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(OBJ)

# Listing of phony targets.
.PHONY : all build prep clean
