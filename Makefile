OUT = xmonitor

CPPFLAGS += -D_POSIX_C_SOURCE=200809L -DVERSION=\"$(VERSION)\"
CFLAGS   += -std=c99 -pedantic -Wall -Wextra
LDLIBS   := -lX11 -lXrandr

PREFIX    ?= /usr/local
BINPREFIX ?= $(PREFIX)/bin

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

all: $(OUT)

debug: CFLAGS += -O0 -g
debug: $(OUT)

xmonitor.o: xmonitor.c
$(OBJ): Makefile

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -p $(OUT) "$(DESTDIR)$(BINPREFIX)"

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)"/$(OUT)

clean:
	rm -f $(OUT) $(OBJ)

.PHONY: all debug install uninstall clean
