MCU				= atmega644p
DUDECPUTYPE		= m644p
PROGRAMMER		= jtag2
LOADCMD			= /usr/bin/sudo /usr/bin/avrdude
DEVPATH			= usb
LOADARG			= -p $(DUDECPUTYPE) -P $(DEVPATH) -c $(PROGRAMMER) -e -U flash:w:

CC				= avr-gcc
OBJCOPY			= avr-objcopy
AVRSIZE			= avr-size

TARGET			= cs.hex
TARGETOUT		= cs.out

SRC				= main.c dcc.c io.c utils.c signal.c scheduler.c ring.c dsl.c sys.c cache.c hash.c
OBJ				= $(SRC:.c=.o)
HDR				= io.h dcc.h utils.h signal.h init.h scheduler.h ring.h dsl.h sys.h cache.h hash.h
INCDIR			=
LIBDIR			=
LIBS			= -lm -Wl,-u,vfprintf -lprintf_flt

# optimize for size
CFLAGS = -g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os -mcall-prologues $(LIBDIR) $(INCDIR)

all: $(TARGET)

$(TARGET): $(TARGETOUT)
	$(AVRSIZE) $(TARGETOUT)
	$(OBJCOPY) -R .eeprom -O ihex $(TARGETOUT) $(TARGET)

$(TARGETOUT): $(OBJ) depend
	$(CC) $(CFLAGS) -o $(TARGETOUT) $(OBJ) $(LIBS)

depend: $(HDR) $(SRC)
	$(CC) -MM $(SRC) >depend

load: $(TARGET)
	$(LOADCMD) $(LOADARG)$(TARGET)

.PHONY: clean doc debug

clean:
	rm -f $(OBJ) $(TARGET) $(TARGETOUT)

doc:
	doxygen

sinclude "depend"
