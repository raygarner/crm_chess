.POSIX:

DEST = /usr/bin
SRC = crm.c
EXES = $(SRC:.c=)

all: $(EXES)

crm:
	$(CC) $(CFLAGS) $@.c -o $@

options:
	@echo "DEST    = $(DEST)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "CC      = $(CC)"
	@echo "SRC     = $(SRC)"
	@echo "EXES    = $(EXES)"

clean:
	rm -f $(EXES) crm.tar.gz

dist: clean
	mkdir -p crm-dist
	cp -R README TODO Makefile $(SRC) crm-dist
	tar -cf - crm-dist | gzip > crm.tar.gz
	rm -rf crm-dist

install: all
	cp -f $(EXES) $(DEST)

uninstall:
	@$(foreach EXE,$(EXES), rm -f $(DEST)/$(EXE))

.PHONY: all options clean dist install uninstall
