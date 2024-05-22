
CFLAGS+=-Wall -pedantic -ansi

crc8_list: crc8_list.c
	$(CC) -o $@ $(CFLAGS) $<

