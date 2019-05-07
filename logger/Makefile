CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=../buffer/buffer.h logger.h ../uart/uart.h ../adsb_parser/adsb_parser.h ../adsb_parser/crc.h
SRCS=$(DEPS:.h=.c) logtest.c
OBJS=$(SRCS:.c=.o)
MAIN=logtest

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS)

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN)

