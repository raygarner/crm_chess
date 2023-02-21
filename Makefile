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
	rm -f $(EXES) crm_chess.tar.gz

dist: clean
	mkdir -p crm_chess-dist
	cp -R README TODO Makefile $(SRC) crm_chess-dist
	tar -cf - crm_chess-dist | gzip > crm_chess.tar.gz
	rm -rf crm_chess-dist

install: all
	cp -f $(EXES) $(DEST)

uninstall:
	@$(foreach EXE,$(EXES), rm -f $(DEST)/$(EXE))

.PHONY: all options clean dist install uninstall
