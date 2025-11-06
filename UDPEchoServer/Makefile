CC = g++
CFLAGS = -Wall -O2
TARGET = UDPEchoServer
SRC = $(TARGET).cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
