CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=../buffer/buffer.h ../logger/logger.h ../uart/uart.h adsb_parser.h crc.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)
MSRC=adsbtest.c
MAIN=adsbtest

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS) $(MSRC)

.c.o: $(DEPS) $(MSRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN)

