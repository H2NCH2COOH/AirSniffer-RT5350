EXEC = cfg_updater
EXEC2 = urldecoder
#CFLAGS += 
#LDFLAGS += -lugpio

.PHONY: default all clean

default: $(EXEC) $(EXEC2)
all: default

OBJECTS = main.o
OBJECTS += ssconfig.o

HEADERS += ssconfig.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(EXEC) $(OBJECTS) $(EXEC2)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@

$(EXEC2): urldecoder.o
	$(CC) $(LDFLAGS) urldecoder.o -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(EXEC)
	-rm -f $(EXEC2)