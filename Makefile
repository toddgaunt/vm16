# See LICENSE file for copyright and license details.

# Project configuration
include config.mk

SRC := \
	arg.h \
	gen.h \
	lex.h \
	log.h \
	zone.h \
	symtab.h \
	txt.h \
	vm16.h \
	gen.c \
	lex.c \
	log.c \
	main.c \
	zone.c \
	symtab.c \
	txt.c \
	vm16.c

DIST := README LICENSE Makefile config.mk $(SRC)

OBJ := $(patsubst %.c, %.o, $(filter %.c, $(SRC)))

# Standard targets
all: vm16

options:
	@echo "Build options:"
	@echo "CFLAGS    = $(CFLAGS)"
	@echo "LDFLAGS   = $(LDFLAGS)"
	@echo "CC        = $(CC)"

test: $(TEST)

check: test
	@for t in $(TEST); do ./$$t -g; done

vcheck: test
	@for t in $(TEST); do valgrind -q ./$$t -v -g; done

dist: $(DIST)
	mkdir vm16-$(VERSION)
	cp -f $(DIST) vm16-$(VERSION)
	tar -czf vm16-$(VERSION).tar.gz vm16-$(VERSION)
	rm -rf vm16-$(VERSION)

sign: dist
	gpg --detach-sign --armor vm16-$(VERSION).tar.gz

install: vm16
	mkdir -p $(BINPREFIX)
	cp -f vm16 $(BINPREFIX)
	mkdir -p $(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < vm16.1 > $(MANPREFIX)/man1/vm16.1
	chmod 644 $(MANPREFIX)/man1/vm16.1

uninstall:
	rm -f $(BINPREFIX)/vm16
	rm -f $(MANPREFIX)/man1/vm16.1

clean:
	rm -f $(OBJ)
	rm -f $(TEST)
	rm -f vm16-$(VERSION).tar.gz
	rm -f vm16

# Object Build Rules
%.o: %.c %.h config.mk
	$(CC) $(CFLAGS) -c -o $@ $<

vm16: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

.PHONY: all options clean check vcheck
