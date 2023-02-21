.POSIX:

DEST = /usr/bin
SRC = craham.c
EXES = $(SRC:.c=)

all: $(EXES)

craham:
	$(CC) $(CFLAGS) $@.c -o $@

options:
	@echo "DEST    = $(DEST)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "CC      = $(CC)"
	@echo "SRC     = $(SRC)"
	@echo "EXES    = $(EXES)"

clean:
	rm -f $(EXES) craham.tar.gz

dist: clean
	mkdir -p craham-dist
	cp -R README TODO Makefile $(SRC) craham-dist
	tar -cf - craham-dist | gzip > craham.tar.gz
	rm -rf craham-dist

install: all
	cp -f $(EXES) $(DEST)

uninstall:
	@$(foreach EXE,$(EXES), rm -f $(DEST)/$(EXE))

.PHONY: all options clean dist install uninstall
