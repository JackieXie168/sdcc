# libc/r2k Makefile


srcdir = .
top_builddir = ../../..

AR_S            = ar -S

LIB_TYPE        = RANLIB

TOPDIR = ../../..

SCC = $(TOPDIR)/bin/sdcc -mr2k
SAS = $(TOPDIR)/bin/sdasrab

# override PORTDIR defined by super (parent) makefile
override PORTDIR = ../build/r2k

include $(srcdir)/../incl.mk

R2K_FLOAT = $(COMMON_FLOAT)

R2K_INT = $(COMMON_INT)

R2K_LONG = $(COMMON_LONG) \
  _divulong.c \
  _mullong.c

R2K_SDCC = $(COMMON_SDCC) \
  _itoa.c \
  _ltoa.c \
  _startup.c \
  sprintf.c \
  vprintf.c

R2KSOURCES = $(addprefix ../,$(R2K_FLOAT) $(R2K_INT) $(R2K_LONG) $(R2K_SDCC))
R2KOBJECTS = $(patsubst %.c,%.rel,$(R2K_FLOAT) $(R2K_INT) $(R2K_LONG) $(R2K_SDCC))

OBJ = divunsigned.rel divsigned.rel divmixed.rel modunsigned.rel modsigned.rel modmixed.rel mul.rel mulchar.rel \
      putchar.rel shift.rel stubs.rel crt0_rle.rel heap.rel fstubs.rel memmove.rel strlen.rel abs.rel crtcall.rel \
      setjmp.rel

LIB = r2k.lib
CC = $(SCC)
AS = $(SAS)
ASFLAGS = -plosgff

CFLAGS = -I$(srcdir)/../../include -I. --std-c99

all: $(PORTDIR)/$(LIB) $(PORTDIR)/crt0.rel

$(PORTDIR)/crt0.rel: crt0.rel
	cp crt0.rel $(PORTDIR)/crt0.rel

$(PORTDIR)/$(LIB): $(OBJ) $(R2KOBJECTS) Makefile
ifeq ($(LIB_TYPE), SDCCLIB)
	rm -f $@; \
	../../../bin/sdcclib -a $@ $(OBJ) $(R2KOBJECTS)
else
  ifeq ($(LIB_TYPE), AR)
	$(AR_S) -cq $@ $(OBJ) $(R2KOBJECTS)
  else
    ifeq ($(LIB_TYPE), RANLIB)
	$(AR_S) -cq $@ $(OBJ) $(R2KOBJECTS)
	$(top_builddir)/bin/asranlib $@
    else
	rm -f $@; \
	for i in $(basename $(OBJ) $(R2KOBJECTS)); do echo $$i >>$@; done
	cp $(OBJ) $(R2KOBJECTS) $(PORTDIR)
    endif
  endif
endif

%.rel: %.c
	$(CC) $(CFLAGS) -c $<

%.rel: %.s
	@# TODO: sdas should place it\'s output in the current dir
	test $(srcdir) = . || cp $< .
	-$(AS) $(ASFLAGS) $(notdir $<)
	test $(srcdir) = . || rm $(notdir $<)

%.rel: ../%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.rel *.sym *.lst *~ $(CLEANSPEC) *.dump* *.asm *.lib

distclean: clean
	rm -f Makefile
