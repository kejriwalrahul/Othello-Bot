# Desdemona Makefile
# Author: Arun Chaganty <arunchaganty@gmail.com>
#

CC=g++
CFLAGS=-Wall -g
CXXFLAGS=$(CFLAGS)
LDFLAGS=
TARGETS=bin/Desdemona

VERSION=0.1

SRCFILES=include/ misc/ src/ tests/ Doxyfile Makefile README 
DISTFILES=bin/desdemona tests/ doc/ README

all: $(TARGETS)

bin/Desdemona: obj/main.o
	if [ ! -e lib ]; then mkdir lib; fi;
	$(CC) $(LDFLAGS) $^ -o $@

obj/main.o: src/main.cpp
	if [ ! -e obj ]; then mkdir obj; fi;
	$(CC) $(CFLAGS) -c $^ -o $@

src-dist: 
	rm -rf Desdemona-src-$(VERSION)
	mkdir Desdemona-src-$(VERSION)
	cp -rf $(SRCFILES) Desdemona-src-$(VERSION)/
	tar -czf Desdemona-src-$(VERSION).tar.gz Desdemona-src-$(VERSION)/
	rm -rf Desdemona-src-$(VERSION)

bin-dist: all
	rm -rf Desdemona-$(VERSION)
	mkdir Desdemona-$(VERSION)
	cp -rf $(DISTFILES) Desdemona-$(VERSION)/
	tar -czf Desdemona-$(VERSION).tar.gz Desdemona-$(VERSION)/
	rm -r Desdemona-$(VERSION)

.PHONY: clean doc

doc: 
	doxygen

clean:
	rm -rf bin/*
	rm -rf lib/*
	rm -rf obj/*
