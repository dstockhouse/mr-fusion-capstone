/***************************************************************************\
 *
 * File:
 * 	vn200.c
 *
 * Description:
 *	Common functionality for VN200, GPS and IMU
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/06/2019
 *
 * Revision 0.2
 * 	Last edited 5/30/2019
 * 	Major overhaul unifying GPS and IMU functionality
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "config.h"
#include "buffer.h"
#include "logger.h"
#include "debuglog.h"
#include "uart.h"
#include "vn200_crc.h"
#include "vn200_gps.h"
#include "vn200_imu.h"

#include "vn200.h"


/**** Function getTimestamp ****
 *
 * Gets a system timestamp as a double and struct timespec
 * Only one of the arguments need not be null
 *
 * Arguments: 
 * 	ts - Pointer to struct timespec to be used in gettime function
 * 	td - Pointer to double to store processed timestamp
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int getTimestamp(struct timespec *ts, double *td) {

    int rc;

    // Used by gettime if input timespec pointer is null
    struct timespec lts;

    if (ts == NULL && td == NULL) {
        return -2;
    }

    // Set to local timespec if input is NULL
    if (ts == NULL) {
        ts = &lts;
    }

    // Get system time
    rc = clock_gettime(CLOCK_REALTIME, ts);
    if (rc) {
        return rc;
    }

    // Return (by reference) time as double
    if (td != NULL) {
        *td = ((double) ts->tv_sec) + ((double) ts->tv_nsec) / 1000000000;
    }

    return 0;

} // getTimestamp(struct timespec *, double *)


/**** Function VN200BaseInit ****
 *
 * Initializes a VN200 IMU/GPS before it is setup for either functionality.
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to initialize
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200BaseInit(VN200_DEV *dev, char *devname, int baud) {

    // Exit on error if invalid pointer
    if (dev == NULL) {
        return -1;
    }

    // If device name not given, use default
    if (devname == NULL) {
        dev->fd = UARTInit(VN200_DEVNAME, baud);
    } else {
        dev->fd = UARTInit(devname, baud);
    }

    if (dev->fd < 0) {
        logDebug(L_INFO, "Couldn't initialize VN200 sensor\n");
        return -2;
    }

    // Initialize the input and output buffers
    BufferEmpty(&(dev->inbuf));
    BufferEmpty(&(dev->outbuf));

#if 0 // TODO REMOVE
    // Initialize packet ring buffer
    dev->ringbuf.start = 0;
    dev->ringbuf.end = 0;
    dev->ringbuf.buf = &(dev->inbuf);
#endif

    return 0;

} // VN200BaseInit(VN200_DEV *)


/**** Function VN200Poll ****
 *
 * Polls the UART file for an initialized VN200 device and populates inbuf with
 * read data. If it detects the start of a packet with '$' it will also move
 * the data into the next available VN200_PACKET in the ring buffer
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to poll
 *
 * Return value:
 *	On success, returns the number of bytes received (may be 0)
 *	On failure, returns a negative number
 */
int VN200Poll(VN200_DEV *dev) {

    int numToRead, numRead, rc, ioctl_status;
    unsigned char uartData[BYTE_BUFFER_LEN];

    // Exit on error if invalid pointer
    if (dev == NULL) {
        return -1;
    }

    // Ensure length of buffer is long enough to hold more data
    if (BufferIsFull(&(dev->inbuf))) {
        logDebug(L_INFO, "VN200Poll: Input buffer is full (%d bytes). Potential loss of data\n",
                BufferLength(&(dev->inbuf)));
        return -2;
    }

    // Some systems don't have this ioctl parameter. Since this isn't a
    // completely necessary check, it can be skipped if not found.
#ifdef FIONREAD
    // Check if how many bytes of UART data available
    rc = ioctl(dev->fd, FIONREAD, &ioctl_status);
    if (rc) {
        logDebug(L_INFO, "%s: VN200Poll: ioctl() failed to fetch FIONREAD\n", strerror(errno));
        // Don't return, not a fatal error
        // return -3;
    }

    logDebug(L_DEBUG, "%d bytes available from UART device...\n", ioctl_status);
#endif

    // Calculate length and pointer to proper position in array
    numToRead = BYTE_BUFFER_LEN - BufferLength(&(dev->inbuf));

    logDebug(L_DEBUG, "Attempting to read %d bytes from uart device...\n", numToRead);

    // Read without blocking from UART device
    numRead = UARTRead(dev->fd, uartData, numToRead);
    logDebug(L_DEBUG, "\tRead %d\n", numRead);

    rc = BufferAddArray(&(dev->inbuf), uartData, numRead);

    if (rc != numRead) {
        logDebug(L_INFO, "WARNING: Couldn't add all bytes read from uart to input buffer\n");
    }

    // Log newly read data to file
    LogUpdate(&(dev->logFile), (char *) uartData, numRead);

    // Return number successfully and saved to buffer (may be 0)
    return rc;

} // VN200Poll(VN200_DEV *)


/**** Function VN200Consume ****
 *
 * Consumes bytes in the input buffer
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to modify
 * 	num - Number of bytes to consume
 *
 * Return value:
 *	On success, returns number of bytes consumed
 *	On failure, returns a negative number
 */
int VN200Consume(VN200_DEV *dev, int num) {

    // Exit on error if invalid pointer
    if (dev == NULL) {
        return -1;
    }

    logDebug(L_VDEBUG, "Attempting to consume %d bytes\n", num);
    num = BufferRemove(&(dev->inbuf), num);
    logDebug(L_VDEBUG, "Consumed %d bytes, %d remaining.\n", num, BufferLength(&(dev->inbuf)));

    return num;

} // VN200Consume(VN200_DEV *, int)


/**** Function VN200FlushInput ****
 *
 * Discards all data recieved through UART
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to modify
 *
 * Return value:
 *	On success, returns number of bytes discarded
 *	On failure, returns a negative number
 */
int VN200FlushInput(VN200_DEV *dev) {

    int num, start, i;

    logDebug(L_VDEBUG, "Flushing input\n");

    // Exit on error if invalid pointer
    if (dev == NULL) {
        return -1;
    }

    start = BufferLength(&(dev->inbuf));

    // Get all waiting characters from UART
    num = VN200Poll(dev);

    logDebug(L_DEBUG, "Flushed VN200 input buffer\n");
    // Print input before discarding
    logDebug(L_VDEBUG, "\tData:\n");
    for (i = start; i < dev->inbuf.length; i++) {
        logDebug(L_VDEBUG, "%c", dev->inbuf.buffer[i]);
    }
    logDebug(L_VDEBUG, "\n");

    // Clear all characters from input buffer
    num = VN200Consume(dev, BufferLength(&(dev->inbuf)));

    return num;

} // VN200FlushInput(VN200_DEV *)


/**** Function VN200Command ****
 *
 * Writes data to the VN200 output buffer, then flushes the output to UART.
 * Follows serial_cmd.m Matlab function
 *
 * Arguments: 
 * 	dev     - Pointer to VN200_DEV instance to modify
 * 	buf     - Pointer to data to be written out
 * 	num     - Number of characters in command
 * 	sendChk - Boolean. True to compute and send checksum with command
 *
 * Return value:
 *	On success, returns number of bytes written
 *	On failure, returns a negative number
 */
int VN200Command(VN200_DEV *dev, char *cmd, int num, int sendChk) {

    const int commandMaxLen = 64;
    char buf[commandMaxLen];
    unsigned char checksum;
    int numWritten, i;

    // Ensure valid pointers
    if (dev == NULL || cmd == NULL) {
        return -1;
    }

    if (sendChk) {

        // Compute and send checksum
        checksum = VN200CalculateChecksum((unsigned char *)cmd, num);
        numWritten = snprintf(buf, commandMaxLen, "$%s*%02X\n", cmd, checksum);

    } else {

        // Send XX instead of checksum
        numWritten = snprintf(buf, commandMaxLen, "$%s*XX\n", cmd);

    } // if (sendChk)

    // Add command string to output buffer
    BufferAddArray(&(dev->outbuf), (unsigned char *) buf, numWritten);

    logDebug(L_VDEBUG, "Output buffer contents: \n");
    for (i = 0; i < dev->outbuf.length; i++) {
        logDebug(L_VDEBUG, "%02X", dev->outbuf.buffer[i]);
    }
    logDebug(L_VDEBUG, "\n");

    // Send output buffer to UART
    numWritten = VN200FlushOutput(dev);

    return numWritten;

} // VN200Command(VN200_DEV &, char *, int, int)


/**** Function VN200FlushOutput ****
 *
 * Writes out data from VN200_DEV struct output buffer to the UART PHY
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to modify
 *
 * Return value:
 *	On success, returns number of bytes written
 *	On failure, returns a negative number
 */
int VN200FlushOutput(VN200_DEV *dev) {

    int numWritten, i;

    // Ensure valid pointers
    if (dev == NULL) {
        return -1;
    }

    // Write output buffer to UART
    numWritten = UARTWrite(dev->fd, dev->outbuf.buffer, dev->outbuf.length);

    logDebug(L_VDEBUG, "Output: \n");
    for (i = 0; i < dev->outbuf.length; i++) {
        logDebug(L_VDEBUG, "%c", dev->outbuf.buffer[i]);
    }
    logDebug(L_VDEBUG, "\n");

    // Remove the data from the output buffer
    BufferRemove(&(dev->outbuf), numWritten);

    return numWritten;

} // VN200FlushOutput(VN200_DEV &)


/**** Function VN200Destroy ****
 *
 * Cleans up an initialized VN200_DEV instance
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to destroy
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200Destroy(VN200_DEV *dev) {

    if (dev == NULL) {
        return -1;
    }

    // Close UART file
    UARTClose(dev->fd);

    // Close log file
    LogClose(&(dev->logFile));

    return 0;

} // VN200Destroy(VN200_DEV &)


/**** Function VN200Init ****
 *
 * Initializes a VN200 UART device for both GPS and IMU functionality
 *
 * Arguments: 
 * 	dev     - Pointer to VN200_DEV instance to initialize
 * 	devname - String name of the device
 * 	fs      - Sampling frequency to initialize the module to
 * 	baud    - Baud rate to configure the UART
 * 	mode    - Initialization mode (found in VN200.h)
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200Init(VN200_DEV *dev, char *devname, int fs, int baud, int mode) {

#define CMD_BUFFER_SIZE 64
    char commandBuf[CMD_BUFFER_SIZE], logBuf[256];
    int commandBufLen, logBufLen;

    char logFileDirName[512];

    // Exit on error if invalid pointer
    if (dev == NULL) {
        return -1;
    }

    // Ensure valid init mode
    if (!(mode == VN200_INIT_MODE_GPS ||
                mode == VN200_INIT_MODE_IMU ||
                mode == VN200_INIT_MODE_BOTH)) {

        logDebug(L_INFO, "VN200Init: Initialization mode 0x%02x not recognized.\n", mode);
        return -2;
    }

    // Ensure valid sample rate (later)

    // Initialize UART for all modes
    dev->baud = baud;
    VN200BaseInit(dev, devname, dev->baud);

    // Initialize log file for raw and parsed data
    // Since multiple log files will be generated for the run, put them in
    // the same directory
    time_t dirtime = time(NULL);
    generateFilename(logFileDirName, 512, &dirtime,
            "log/data/VN200", "RUN", "d");
    LogInit(&(dev->logFile), logFileDirName, "VN200", LOG_FILEEXT_LOG);
    logDebug(L_INFO, "Logging to directory %s\n", logFileDirName);

    // If GPS enabled, init GPS log file
    if (mode & VN200_INIT_MODE_GPS) {

        // Init csv file
        LogInit(&(dev->logFileGPSParsed), logFileDirName, "VN200_GPS", LOG_FILEEXT_CSV);

        // Write header to CSV data
        logBufLen = snprintf(logBuf, 256, "gpstime,week,gpsfix,numsats,lat,lon,alt,velx,vely,velz,nacc,eacc,vacc,sacc,tacc,timestamp\n");
        LogUpdate(&(dev->logFileGPSParsed), logBuf, logBufLen);

    }

    // If IMU enabled, init IMU log file
    if (mode & VN200_INIT_MODE_IMU) {

        // Init csv file
        LogInit(&(dev->logFileIMUParsed), logFileDirName, "VN200_IMU", LOG_FILEEXT_CSV);

        // Write header to CSV data
        logBufLen = snprintf(logBuf, 256, "compx,compy,compz,accelx,accely,accelz,gyrox,gyroy,gyroz,temp,baro,timestamp\n");
        LogUpdate(&(dev->logFileIMUParsed), logBuf, logBufLen);

    }


    /**** Initialize VN200 through UART commands ****/

    // Ensure Baud rate is expected rate
    UARTSetBaud(dev->fd, 57600);
    char *baudCommandString = "VNWRG,05,115200";
    VN200Command(dev, baudCommandString, strlen(baudCommandString), 0);
    VN200FlushOutput(dev);
    usleep(100000);
    UARTSetBaud(dev->fd, 115200);
    VN200FlushInput(dev);
    usleep(100000);

    // Request VN200 serial number
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNRRG,03");
    VN200Command(dev, commandBuf, commandBufLen, 0);
    VN200FlushOutput(dev);
    usleep(100000);
    VN200FlushInput(dev);
    usleep(100000);

    // Disable asynchronous output
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,0");
    VN200Command(dev, commandBuf, commandBufLen, 0);
    VN200FlushOutput(dev);
    usleep(100000);
    VN200FlushInput(dev);
    usleep(100000);

    if (mode == VN200_INIT_MODE_GPS) {

        // Enable asynchronous GPS data output
        commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,20");

    } else if (mode == VN200_INIT_MODE_IMU) {

        // Enable asynchronous IMU data output
        commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,19");

    } else { // BOTH

        /*
        // IMU/GPS Sample frequency are not the same, so use default frequencies
        // already set for both
        dev->fs = 0;
        */

        // Enable both GPS and IMU output
        commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,248");
    }


    // Send mode command to UART
    VN200Command(dev, commandBuf, commandBufLen, 0);
    VN200FlushOutput(dev);
    usleep(100000);
    VN200FlushInput(dev);
    usleep(100000);

    // Set sampling frequency (register 07)
    dev->fs = fs;
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s%02d", "VNWRG,07,", dev->fs);
    VN200Command(dev, commandBuf, commandBufLen, 1);
    VN200FlushOutput(dev);
    usleep(100000);
    VN200FlushInput(dev);
    usleep(100000);

    if (devname != NULL) {
        logDebug(L_INFO, "Finished configuring UART for device %s\n", devname);
    } else {
        logDebug(L_INFO, "Finished configuring UART for default device\n");
    }

    // Clear input buffer to prevent parsing "latent" data (temporary)
    VN200FlushInput(dev);

    return 0;

} // VN200Init(VN200_DEV *, int)

