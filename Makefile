CC = gcc
CFLAGS = -W -Wall
TARGET = 20181666.out
OBJECTS = memory.o opcode.o assemble.o linker.o 20181666.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm -f $(OBJECTS) $(TARGET)
