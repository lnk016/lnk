CC=gcc
CFLAGS=-Wall -Wextra -O2
TARGET=lnk
PREFIX=/usr/local

all:
	$(CC) $(CFLAGS) main.c -o $(TARGET)

install: all
	install -Dm755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)
