CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=buffer.h ../logger/logger.h ../uart/uart.h ../adsb/adsb_parser.h ../adsb/crc.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)
MSRC=buftest.c
MAIN=buftest

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS) $(MSRC)

.c.o: $(DEPS) $(MSRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN)

