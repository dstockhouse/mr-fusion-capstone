CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=../buffer/buffer.h logger.h ../uart/uart.h ../adsb_parser/adsb_parser.h ../adsb_parser/crc.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)
MSRC=logtest.c
MAIN=logtest

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS) $(MSRC)

.c.o: $(DEPS) $(MSRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN)

