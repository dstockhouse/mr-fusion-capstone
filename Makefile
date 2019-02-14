CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=buffer.h uart.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)
MAIN=adsbtest

$(MAIN): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(OBJS)

buftest: $(OBJS) $(DEPS) buftest.c
	$(CC) $(CFLAGS) -o $@ buftest.c $(OBJS)

uarttest: $(OBJS) $(DEPS) uarttest.c
	$(CC) $(CFLAGS) -o $@ uarttest.c $(OBJS)

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(MAIN) buftest uarttest
