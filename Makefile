
CFLAGS+=-Wall -pedantic -ansi

crc8: crc8.c
	$(CC) -o $@ $(CFLAGS) $<

