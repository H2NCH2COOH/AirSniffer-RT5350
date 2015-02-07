EXEC = cfg_updater
#CFLAGS += 
#LDFLAGS += -lugpio

.PHONY: default all clean

default: $(EXEC) $(WELCOME)
all: default

OBJECTS = main.o
OBJECTS += ssconfig.o

HEADERS += ssconfig.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(EXEC) $(OBJECTS) $(WELCOME) $(WELCOME_OBJ)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@
	
clean:
	-rm -f *.o
	-rm -f $(EXEC)