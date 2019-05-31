/***************************************************************************\
 *
 * File:
 * 	VN200_GPS_and_IMU.c
 *
 * Description:
 * 	Interfaces with a VN200 device outputting both GPS and IMU data
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/30/2019
 *
\***************************************************************************/

#include "VN200.h"
#include "VN200_GPS.h"
#include "VN200_IMU.h"
#include "VN200_GPS_and_IMU.h"

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"
#include "crc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
