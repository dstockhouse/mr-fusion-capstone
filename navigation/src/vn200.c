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
#include "utils.h"
#include "uart.h"
#include "vn200_crc.h"
#include "vn200_gps.h"
#include "vn200_imu.h"

#include "vn200.h"


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

    int numToRead, numRead, rc;
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
    int ioctl_status;
    rc = ioctl(dev->fd, FIONREAD, &ioctl_status);
    if (rc) {
        logDebug(L_INFO, "%s: VN200Poll: ioctl() failed to fetch FIONREAD\n", strerror(errno));
        // Don't return, not a fatal error
        // return -3;
    }

    logDebug(L_VVDEBUG, "%d bytes available from UART device...\n", ioctl_status);
#endif

    // Calculate length and pointer to proper position in array
    numToRead = BYTE_BUFFER_MAX_LEN - BufferLength(&(dev->inbuf));

    logDebug(L_VDEBUG, "Attempting to read %d bytes from uart device...\n", numToRead);

    // Read without blocking from UART device
    numRead = UARTRead(dev->fd, uartData, numToRead);
    logDebug(L_VDEBUG, "\tRead %d\n", numRead);

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

    // Write command to UART device
    UARTWrite(dev->fd, (unsigned char *) buf, numWritten);
    // logDebug(L_INFO, "VN200Command: %s", buf);

    return numWritten;

} // VN200Command(VN200_DEV &, char *, int, int)


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
 * 	dev      - Pointer to VN200_DEV instance to initialize
 * 	devname  - String name of the device
 * 	fs       - Sampling frequency to initialize the module to
 * 	baud     - Baud rate to configure the UART
 * 	mode     - Initialization mode (found in VN200.h)
 * 	initTime - Timestamp to put on log directory and files
 * 	key      - Numeric key to identify log files together
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200Init(VN200_DEV *dev, char *devname, char *logDirName, int fs, int baud, int mode,
        time_t *initTime, unsigned key) {

#define CMD_BUFFER_SIZE 64
    char commandBuf[CMD_BUFFER_SIZE], logBuf[256];
    int commandBufLen, logBufLen;

    char logDirNameTemp[512];
    time_t initTimeTemp;

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

    // If no time provided, use current time
    if (initTime == NULL) {
        initTimeTemp = time(NULL);
        initTime = &initTimeTemp;
    }

    // Ensure valid sample rate (later)

    // Initialize UART for all modes
    dev->baud = baud;
    VN200BaseInit(dev, devname, dev->baud);

    // Initialize log file for raw and parsed data
    // Since multiple log files will be generated for the run, put them in
    // the same directory
    if (logDirName == NULL) {
        // If no directory name is provided, make one yourself
        generateFilename(logDirNameTemp, 512, initTime,
                "log/data/VN200", "RUN", key, "d");
        logDirName = logDirNameTemp;
    }

    LogInit(&(dev->logFile), logDirName, "VN200", initTime, key, LOG_FILEEXT_LOG);
    logDebug(L_INFO, "Logging VN200 data to directory %s\n", logDirName);

    // If GPS enabled, init GPS log file
    if (mode & VN200_INIT_MODE_GPS) {

        // Init csv file
        LogInit(&(dev->logFileGPSParsed), logDirName, "VN200_GPS", initTime, key, LOG_FILEEXT_CSV);

        // Write header to CSV data
        logBufLen = snprintf(logBuf, 256, "gpstime,week,gpsfix,numsats,posx,posy,posz,velx,vely,velz,xacc,yacc,zacc,sacc,tacc,timestamp\n");
        LogUpdate(&(dev->logFileGPSParsed), logBuf, logBufLen);

    }

    // If IMU enabled, init IMU log file
    if (mode & VN200_INIT_MODE_IMU) {

        // Init csv file
        LogInit(&(dev->logFileIMUParsed), logDirName, "VN200_IMU", initTime, key, LOG_FILEEXT_CSV);

        // Write header to CSV data
        logBufLen = snprintf(logBuf, 256, "compx,compy,compz,accelx,accely,accelz,gyrox,gyroy,gyroz,temp,baro,timestamp\n");
        LogUpdate(&(dev->logFileIMUParsed), logBuf, logBufLen);

    }


    /**** Initialize VN200 through UART commands ****/

    // Ensure Baud rate is expected rate
    UARTSetBaud(dev->fd, 57600);
    char *baudCommandString = "VNWRG,05,115200";
    VN200Command(dev, baudCommandString, strlen(baudCommandString), 0);
    UARTSetBaud(dev->fd, 115200);

    // Request VN200 serial number
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNRRG,03");
    VN200Command(dev, commandBuf, commandBufLen, 0);

    // Disable asynchronous output
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,0");
    VN200Command(dev, commandBuf, commandBufLen, 0);

    // Set sampling frequency (register 07)
    dev->fs = fs;
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s%02d", "VNWRG,07,", dev->fs);
    VN200Command(dev, commandBuf, commandBufLen, 1);

    // Save register settings
    char *writeNVMCommandString = "VNWNV";
    VN200Command(dev, writeNVMCommandString, strlen(writeNVMCommandString), 0);

    // Reset device with these settings
    char *resetCommandString = "VNRST";
    VN200Command(dev, resetCommandString, strlen(resetCommandString), 0);


    // Wait 200ms for device to reset
    usleep(1000000);

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

    if (devname != NULL) {
        logDebug(L_INFO, "Finished configuring UART for device %s\n", devname);
    } else {
        logDebug(L_INFO, "Finished configuring UART for default device\n");
    }

    // Clear input buffer to prevent parsing "latent" data (temporary)
    VN200FlushInput(dev);

    return 0;

} // VN200Init(VN200_DEV *, int)

