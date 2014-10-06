EXEC = AirSniffer
#CFLAGS += 
LDFLAGS += -lugpio

.PHONY: default all clean

default: $(EXEC)
all: default

OBJECTS = main.o
#OBJECTS += net.o
#OBJECTS += sensor.o
OBJECTS += datalcd.o
OBJECTS += lcd.o
OBJECTS += sshttp.o
OBJECTS += ssconfig.o
OBJECTS += xively.o

HEADERS = config_key.h
#HEADERS += pipecmd.h
#HEADERS += net.h
#HEADERS += sensor.h
HEADERS += sshttp.h
HEADERS += ssconfig.h
HEADERS += xively.h
HEADERS += lcd.h
HEADERS += datalcd.h
HEADERS += gpio_pin.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(EXEC) $(OBJECTS)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(EXEC)