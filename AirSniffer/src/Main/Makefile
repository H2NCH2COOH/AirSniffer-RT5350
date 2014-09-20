EXEC = AirSniffer
#CFLAGS += 
LDFLAGS += -lugpio

.PHONY: default all clean

default: $(EXEC)
all: default

OBJECTS = main.o
#OBJECTS += net.o
#OBJECTS += sensor.o
OBJECTS += sshttp.o
OBJECTS += ssconfig.o
OBJECTS += xively.o

HEADERS = config_key.h
#HEADERS += pipecmd.h
#HEADERS += net.h
#HEADERS += sensor.h
OBJECTS += sshttp.h
OBJECTS += ssconfig.h
OBJECTS += xively.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(EXEC) $(OBJECTS)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(EXEC)