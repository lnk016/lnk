CC=gcc
CFLAGS=-Wall -Wextra -O2
TARGET=lnk

all:
	$(CC) $(CFLAGS) main.c -o $(TARGET)

clean:
	rm -f $(TARGET)
