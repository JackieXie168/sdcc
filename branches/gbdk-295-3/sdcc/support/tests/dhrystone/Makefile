# Simple Makefile for dhrystone and sdcc

PROC = z80
PORT = consolez80

TOPDIR = ../../..
CC = $(TOPDIR)/bin/sdcc

LIB = /home/michaelh/projects/gbdk-lib

# -DNOENUM is here to make the results more predictable
CFLAGS = -mz80 -v -I$(LIB)/include
CFLAGS += -DREG= -DNOSTRUCTASSIGN -DNOENUM
LIBDIR = /home/michaelh/projects/gbdk-lib/build/small/asxxxx
LD = $(TOPDIR)/bin/link-$(PROC)
LDFLAGS = -v
AS = as-$(PROC)

OBJ = dhry.o

all: dhry

dhry: dhry.s $(OBJ)
	$(LD) -n -- -i dhry.ihx -m -b_DATA=0x8000 -b_CODE=0x200 -k$(LIBDIR)/consolez80/ -lconsolez80.lib -k$(LIBDIR)/z80/ -lz80.lib $(LIBDIR)/consolez80/crt0.o $(OBJ)
	cat dhry.ihx | ../../makebin/makebin  > dhry.rom

dhry.s: dhry.c
	$(CC) $(CFLAGS) -S -o dhry.s dhry.c

native:
	gcc -g -O2 -DREG= -DNOSTRUCTASSIGN -DNOENUM -o dhry dhry.c

dhry.c:

clean:
	rm -f *~ dhry *.o *.gb
