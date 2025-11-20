CC = gcc
CFLAGS = -Wall -g
LIBS = $(shell pkg-config --cflags --libs gtk+-3.0)
TARGET = unified-launcher
SRCDIR = .
SOURCES = $(SRCDIR)/main.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin

run: $(TARGET)
	./$(TARGET)