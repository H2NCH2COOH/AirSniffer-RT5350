EXEC = wifi_setup_agent
#CFLAGS += 
#LDFLAGS += 

.PHONY: default all clean

default: $(EXEC)
all: default

OBJECTS = main.o
OBJECTS += wpa_ctrl.o
OBJECTS += common.o
OBJECTS += os_unix.o

HEADERS = wpa_ctrl.h
HEADERS += includes.h
HEADERS += build_config.h
HEADERS += common.h
HEADERS += os.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(EXEC) $(OBJECTS)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(EXEC)