CC = gcc
CFLAGS = -Wall -O2 -Wno-unused-result
TARGET = servidor
SRC = servidor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)