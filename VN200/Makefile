CC=gcc
CFLAGS=-g -Wall
LIBS=
DEPS=../buffer/buffer.h ../logger/logger.h ../uart/uart.h crc.h VN200.h VN200_GPS.h VN200_IMU.h
SRCS=$(DEPS:.h=.c)
OBJS=$(SRCS:.c=.o)

all: gpstest imutest


gpstest: $(OBJS) gpstest.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

imutest: $(OBJS) imutest.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) gpstest imutest

