/****************************************************************************\
 *
 * File:
 * 	vn200_gps.c
 *
 * Description:
 * 	Interfaces with an GPS reciever connected to serial port through USB
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 4/16/2019
 *
 * Revision 0.2
 * 	Last edited 5/06/2019
 *
 \***************************************************************************/

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

#include "config.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "vn200_crc.h"
#include "vn200.h"

#include "vn200_gps.h"


/**** Function VN200GPSInit ****
 *
 * Initializes a VN200 UART device for GPS functionality
 *
 * Arguments: 
 * 	dev        - Pointer to VN200_DEV instance to initialize
 * 	fs         - Sampling frequency to initialize the module to
 * 	devname    - Name of the UART device (set NULL to use default)
 * 	logDirName - Name of the directory to log to
 * 	initTime   - Timestamp to put on log directory and files
 * 	key        - Numeric key to identify log files together
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200GPSInit(VN200_DEV *dev, char *devname, char *logDirName, int fs, time_t *initTime, unsigned key) {

    return VN200Init(dev, devname, logDirName, fs, VN200_BAUD, VN200_INIT_MODE_GPS, initTime, key);

} // VN200GPSInit(VN200_DEV *, char *, int)


/**** Function VN200GPSPacketParse ****
 *
 * Parses data from input buffer, assuming GPS
 *
 * Example packet:
 * 	$VNGPS,342123.000168,1890,3,05,+34.61463270,-112.45087270,+01559.954,+000.450,+000.770,-001.290,+002.940,+005.374,+007.410,+001.672,2.10E-08*23
 *
 * 	The input buffer must start at first character after "$VNGPS," and the
 * 	length is the number of characters until *XX
 *
 * Arguments: 
 * 	buf  - Pointer to character buffer to parse from
 * 	len  - Length of character buffer
 * 	data - Pointer to GPS_DATA instance to store parsed data
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200GPSPacketParse(unsigned char *buf, int len, GPS_DATA *data) {

    int i, rc;

    // Exit on error if invalid pointer
    if(buf == NULL || data == NULL) {
        return -1;
    }

    // Exit if invalid length
    if(len < 0) {
        return -2;
    }

    // Get a timestamp for the data
    // (not completely accurate but should be within ~100ms)
    getTimestamp(NULL, &(data->timestamp));

    logDebug(L_VVDEBUG, "\n\n%s - Data in buffer:\n", __func__);
    for(i = 0; i < len; i++) {
        logDebug(L_VVDEBUG, "%c", buf[i]);
    }
    logDebug(L_VVDEBUG, "\n\n");

    // Scan for fields in packet string
    rc = sscanf((char *) buf, "%lf,%hd,%hhd,%hhd,%lf,%lf,%lf,%f,%f,%f,%f,%f,%f,%f,%f",
            &(data->time), &(data->week), &(data->GpsFix), &(data->NumSats),
            &(data->PosX), &(data->PosY), &(data->PosZ),
            &(data->VelX), &(data->VelY), &(data->VelZ),
            &(data->PosAccX), &(data->PosAccY), &(data->PosAccZ),
            &(data->SpeedAcc), &(data->TimeAcc));
    if(rc < 15) {
        logDebug(L_INFO, "%s: Didn't match entire formatted string: %d\n", __func__, rc);

        // Malformed packet, return to indicate not fully parsed
        return -3;
    }

    return len;

} // VN200GPSPacketParse(unsigned char *, int, GPS_DATA *)


int VN200GPSLogParsed(LOG_FILE *log, GPS_DATA *data) {

    char logBuf[512];
    int logBufLen;

    // Log parsed data to file in CSV format
    logBufLen = snprintf(logBuf, 512, "%.4lf,%hd,%hhd,%hhd,%.3lf,%.3lf,%.3lf,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.2e,%.4lf\n",
            data->time, data->week, data->GpsFix, data->NumSats,
            data->PosX, data->PosY, data->PosZ,
            data->VelX, data->VelY, data->VelZ,
            data->PosAccX, data->PosAccY, data->PosAccZ,
            data->SpeedAcc, data->TimeAcc, data->timestamp);

    // Update log file
    LogUpdate(log, logBuf, logBufLen);

    return 0;

} // VN200GPSLogParsed(VN200_DEV *, GPS_DATA *)

