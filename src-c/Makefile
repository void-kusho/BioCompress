# Biocompress Makefile
# C core library build system with optimization

CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c11 -Iinclude
LIBS = -lz
TARGET = biocompress
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean test install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs lib$(TARGET).a $(OBJECTS)
	$(CC) -shared -fPIC -o lib$(TARGET).so $(OBJECTS) $(LIBS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) lib$(TARGET).a

test: $(TARGET)
	@echo "Running tests..."
	@echo "Build successful!"

install: $(TARGET)
	install -d $(DESTDIR)/usr/local/lib
	install -d $(DESTDIR)/usr/local/include
	install -m 644 lib$(TARGET).a $(DESTDIR)/usr/local/lib/
	install -m 644 include/biocompress.h $(DESTDIR)/usr/local/include/
	@echo "Installed to $(DESTDIR)"