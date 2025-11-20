CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`
TARGET = makefile-generator
SRC = src/main.c src/gui.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) dependencies.txt

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

deps:
	@echo "检查系统依赖..."
	@pkg-config --exists gtk+-3.0 && echo "GTK+3 已安装" || echo "请安装 GTK+3 开发包"

.PHONY: all clean install deps