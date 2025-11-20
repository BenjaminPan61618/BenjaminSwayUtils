CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 glib-2.0 gio-2.0 json-glib-1.0 ayatana-appindicator3-0.1` -g -Wall
LIBS = `pkg-config --libs gtk+-3.0 glib-2.0 gio-2.0 json-glib-1.0 ayatana-appindicator3-0.1`
SRC = main.c window_manager.c file_classifier.c tray_icon.c context_menu.c \
      desktop_monitor.c settings.c custom_categories.c ui_components.c
OBJ = $(SRC:.c=.o)
TARGET = desktop-organizer

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean
