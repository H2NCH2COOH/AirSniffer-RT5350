EXEC = AirSniffer
WELCOME = Welcome
#CFLAGS += 
LDFLAGS += -lugpio

.PHONY: default all clean

default: $(EXEC) $(WELCOME)
all: default

OBJECTS = main.o
#OBJECTS += net.o
#OBJECTS += sensor.o
#OBJECTS += datalcd.o
OBJECTS += lcd.o
OBJECTS += img.o
OBJECTS += sshttp.o
OBJECTS += ssconfig.o
OBJECTS += xively.o
OBJECTS += upload.o

WELCOME_OBJ = welcome.o
WELCOME_OBJ += lcd.o
#WELCOME_OBJ += datalcd.o
WELCOME_OBJ += img.o

HEADERS = config_key.h
#HEADERS += pipecmd.h
#HEADERS += net.h
#HEADERS += sensor.h
HEADERS += sshttp.h
HEADERS += ssconfig.h
HEADERS += xively.h
HEADERS += lcd.h
#HEADERS += datalcd.h
HEADERS += gpio_pin.h
HEADERS += img.h
HEADERS += main.h
HEADERS += upload.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(EXEC) $(OBJECTS) $(WELCOME) $(WELCOME_OBJ)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@

$(WELCOME): $(WELCOME_OBJ)
	$(CC) $(LDFLAGS) $(WELCOME_OBJ) -Wall -o $@
	
clean:
	-rm -f *.o
	-rm -f $(EXEC)
	-rm -f $(WELCOME)