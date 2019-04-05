CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=../buffer/buffer.h ../logger/logger.h uart.h ../adsb_parser/adsb_parser.h ../adsb_parser/crc.h ../pingusb.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)
MSRC=uarttest.c
MAIN=uarttest

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS) $(MSRC)

.c.o: $(DEPS) $(MSRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN)

