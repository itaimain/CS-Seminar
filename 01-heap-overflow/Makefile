CC = gcc
CFLAGS = -g -O0 -no-pie -Wall -Wextra -Wno-stringop-overflow
TARGET = victim

all: $(TARGET)

$(TARGET): victim.c
	$(CC) $(CFLAGS) -o $(TARGET) victim.c

run: $(TARGET)
	./$(TARGET)

exploit: $(TARGET)
	python3 exploit.py

clean:
	rm -f $(TARGET)

.PHONY: all run exploit clean
