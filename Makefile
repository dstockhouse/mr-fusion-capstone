CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=buffer.h logger.h uart.h ADS_B.h crc.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)
MAIN=test

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS)

buftest: $(OBJS) $(DEPS) buftest.c
	$(CC) $(CFLAGS) -o $@ buftest.c $(OBJS)

logtest: $(OBJS) $(DEPS) logtest.c
	$(CC) $(CFLAGS) -o $@ logtest.c $(OBJS)

uarttest: $(OBJS) $(DEPS) uarttest.c
	$(CC) $(CFLAGS) -o $@ uarttest.c $(OBJS)

adsbtest: $(OBJS) $(DEPS) adsbtest.c
	$(CC) $(CFLAGS) -o $@ adsbtest.c $(OBJS)

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN) buftest logtest uarttest
