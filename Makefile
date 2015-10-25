CC=gcc
CFLAGS= -std=c99 -g -Wall -pedantic

all: lzw

clean: 
	$(RM) lzw.o hash.o lzw

lzw: lzw.o hash.o
	$(CC) $(CFLAGS) -o $@ $^

lzw.o: lzw.c lzw.h hash.h
	$(CC) $(CFLAGS) -c $<

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c $<




