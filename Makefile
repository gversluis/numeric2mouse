CC = gcc
CFLAGS = -Wall -O2
LIBS = -lyaml

TARGET = numeric2mouse
CONFIG = /etc/numeric2mouse.yaml

all: $(TARGET)

$(TARGET): numeric2mouse.c
	$(CC) $(CFLAGS) -o $(TARGET) numeric2mouse.c $(LIBS)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	@if [ ! -f $(CONFIG) ]; then \
		echo "Installing default config to $(CONFIG)"; \
		install -m 644 numeric2mouse.yaml $(CONFIG); \
	else \
		echo "Config file $(CONFIG) already exists, not overwriting"; \
	fi

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Note: Not removing $(CONFIG) - remove manually if needed"

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
