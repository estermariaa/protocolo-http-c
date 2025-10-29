CC = gcc
CFLAGS = -Wall -O2 -Wno-unused-result
TARGET = cliente
SRC = cliente.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)