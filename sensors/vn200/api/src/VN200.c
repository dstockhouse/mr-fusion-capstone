/***************************************************************************\
 *
 * File:
 * 	VN200.c
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

#include "control.h"
#include "buffer.h"
#include "debuglog.h"
#include "logger.h"
#include "uart.h"
#include "VN200_CRC.h"
#include "VN200_GPS.h"
#include "VN200_IMU.h"
#include "VN200Packet.h"
#include "VN200.h"


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

    if(ts == NULL && td == NULL) {
        return -2;
    }

    // Set to local timespec if input is NULL
    if(ts == NULL) {
        ts = &lts;
    }

    // Get system time
    rc = clock_gettime(CLOCK_REALTIME, ts);
    if(rc) {
        return rc;
    }

    // Return (by reference) time as double
    if(td != NULL) {
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

    int i;

    // Exit on error if invalid pointer
    if(dev == NULL) {
        return -1;
    }

    // If device name not given, use default
    if(devname == NULL) {
        dev->fd = UARTInit(VN200_DEVNAME, baud);
    } else {
        dev->fd = UARTInit(devname, baud);
    }

    if(dev->fd < 0) {
        logDebug("Couldn't initialize VN200 sensor\n");
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

    int numToRead, numRead, startIndex, rc, ioctl_status;
    char *startBuf, tempBuf[BYTE_BUFFER_LEN];

    // Exit on error if invalid pointer
    if(dev == NULL) {
        return -1;
    }

    // Ensure length of buffer is long enough to hold more data
    if(dev->inbuf.length >= BYTE_BUFFER_LEN) {
        logDebug("VN200Poll: Input buffer is full (%d bytes)\n",
                dev->inbuf.length);
        return -2;
    }

    // Check if UART data available
    rc = ioctl(dev->fd, FIONREAD, &ioctl_status);
    if(rc) {
        perror("VN200Poll: ioctl() failed");
        // return -3;
    }
#ifdef STANDARD_DEBUG
    logDebug("%d bytes avail...\n", ioctl_status);
#endif

    // Calculate length and pointer to proper position in array
    numToRead = BYTE_BUFFER_LEN - dev->inbuf.length;
    startIndex = dev->inbuf.length;
    startBuf = &(dev->inbuf.buffer[startIndex]);
#ifdef VERBOSE_DEBUG
    logDebug("Poll: startBuf is %p\n", startBuf);
#endif
    // startBuf[0] = 2;

#ifdef STANDARD_DEBUG
    logDebug("Attempting to read %d bytes from uart device...\n", numToRead);
#endif

    // Read without blocking from UART device
    numRead = UARTRead(dev->fd, startBuf, numToRead);
    // numRead = UARTRead(dev->fd, tempBuf, numToRead);
#ifdef STANDARD_DEBUG
    logDebug("\tRead %d\n", numRead);
#endif

    // Log newly read data to file
    LogUpdate(&(dev->logFile), startBuf, numRead);
    // LogUpdate(&(dev->logFile), tempBuf, numRead);

    // memcpy(&(dev->inbuf.buffer[dev->inbuf.length]), tempBuf, numRead);

    // Update input buffer endpoint
    dev->inbuf.length += numRead;

#if 0 // TODO REMOVE

    // Update ring buffer endpoints, generating new packets as needed
#ifdef STANDARD_DEBUG
    logDebug("Updating endpoints\n");
#endif
    VN200PacketRingBufferUpdateEndpoints(&(dev->ringbuf));

#endif

    // This is now handled by UpdateEndpoints function
#if 0
    // Populate most recent packet with data and/or start a new packet
    for(i = 0; i < numRead; i++) {

        // If start of packet, create new packet
        if(startBuf[i] == '$') {

            // Initialize new packet
            rc = VN200PacketRingBufferAddPacket(&(dev->ringbuf));
            if(rc < 1) {
                logDebug("VN200Parse: Couldn't add packet to ring buffer\n");
                return -1;
            }

        }

        // If ring buffer is not empty
        // (there is a partially complete packet)
        if(!VN200RingBufferIsEmpty(&(dev->ringbuf))) {

            // Add character to packet data buffer
            rc = VN200PacketRingBufferAddData(&(dev->ringbuf), startBuf[i]);
            if(rc < 1) {
                return rc;
            }

        } else {

#ifdef VERBOSE_DEBUG
            // Printout incomplete packet data
            logDebug("%c", startBuf[i]);
#endif

        }

    } // for(i < numRead)
#endif

    // Return number successfully read (may be 0)
    return numRead;

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

    int i, packetsConsumed = 0;

    // Exit on error if invalid pointer
    if(dev == NULL) {
        return -1;
    }

#ifdef VERBOSE_DEBUG
    logDebug("Attempting to consume %d bytes\n", num);
#endif
    num = BufferRemove(&(dev->inbuf), num);
#ifdef STANDARD_DEBUG
    logDebug("Consumed %d bytes, %d remaining.\n", num, dev->inbuf.length);
#endif


#if 0 // TODO REMOVE

    logDebug("Updating packet indices.\n");
    // Loop through ring buffer to adjust every packet
    VN200_PACKET_RING_BUFFER *ringbuf = &(dev->ringbuf);
    VN200_PACKET *packet;
    for(i = ringbuf->start;
            i != ringbuf->end;
            i = (i + 1) % VN200_PACKET_RING_BUFFER_SIZE) {

#ifdef VERBOSE_DEBUG
        logDebug("\tIn loop. s=%d, e=%d, i=%d...\n",
                ringbuf->start, ringbuf->end, i);
#endif

        // Pointer to current packet
        packet = &(ringbuf->packets[i]);

        // Move start and end indices backwards by length of that packet
        packet->startIndex -= num;
        packet->endIndex -= num;

#ifdef VERBOSE_DEBUG
        logDebug("\tEnd of loop. si=%d, ei=%d...\n",
                packet->startIndex, packet->endIndex);
#endif
        // If index is below 0 the packet has been (at least partially) consumed
        if(packet->startIndex < 0 || packet->endIndex < 0) {
            packetsConsumed += VN200PacketRingBufferRemovePacket(ringbuf);
        }

    }

#ifdef STANDARD_DEBUG
    logDebug("Consumed %d packets\n", packetsConsumed);
#endif

#endif

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

#ifdef STANDARD_DEBUG
    logDebug("Flushing input\n");
#endif

    // Exit on error if invalid pointer
    if(dev == NULL) {
        return -1;
    }

    start = dev->inbuf.length;

    // Get all waiting characters from UART
    num = VN200Poll(dev);

    logDebug("Flushed input\n");
#ifdef VERBOSE_DEBUG
    // Print input before discarding
    logDebug("\tData:\n");
    for(i = start; i < dev->inbuf.length; i++) {
        logDebug("%c", dev->inbuf.buffer[i]);
    }
    logDebug("\n");
#endif

    // Clear all characters from input buffer
    num = VN200Consume(dev, num);

    return num;

} // VN200FlushInput(VN200_DEV *)


/**** Function VN200Command ****
 *
 * Writes data to the VN200 output buffer, then flushes the output to UART.
 * Follows serial_cmd.m Matlab function
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to modify
 * 	buf - Pointer to data to be written out
 * 	num - Number of bytes to write
 *
 * Return value:
 *	On success, returns number of bytes written
 *	On failure, returns a negative number
 */
int VN200Command(VN200_DEV *dev, char *cmd, int num, int sendChk) {

    char buf[64];
    unsigned char checksum;
    int numWritten, i;

    // Ensure valid pointers
    if(dev == NULL || cmd == NULL) {
        return -1;
    }


    if(sendChk) {

        // Compute and send checksum
        // checksum = VN200CalculateChecksum(cmd, num);
        checksum = VN200CalculateChecksum(cmd, strlen(cmd));
        numWritten = snprintf(buf, 64, "$%s*%02X\n", cmd, checksum);

    } else {

        // Send XX instead of checksum
        numWritten = snprintf(buf, 64, "$%s*XX\n", cmd);

    } // if(sendChk)

    // Add command string to output buffer
    BufferAddArray(&(dev->outbuf), buf, numWritten);

#ifdef VERBOSE_DEBUG
    logDebug("Output buffer contents: \n");
    for(i = 0; i < dev->outbuf.length; i++) {
        logDebug("%02X", dev->outbuf.buffer[i]);
    }
    logDebug("\n");
#endif

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
    if(dev == NULL) {
        return -1;
    }

    // Write output buffer to UART
    numWritten = UARTWrite(dev->fd, dev->outbuf.buffer, dev->outbuf.length);

#ifdef VERBOSE_DEBUG
    logDebug("Output: \n");
    for(i = 0; i < dev->outbuf.length; i++) {
        logDebug("%c", dev->outbuf.buffer[i]);
    }
    logDebug("\n");
#endif

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

    if(dev == NULL) {
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
 * 	dev  - Pointer to VN200_DEV instance to initialize
 * 	fs   - Sampling frequency to initialize the module to
 * 	baud - Baud rate to configure the UART
 * 	mode - Initialization mode (found in VN200.h)
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200Init(VN200_DEV *dev, int fs, int baud, int mode) {

#define CMD_BUFFER_SIZE 64
    char commandBuf[CMD_BUFFER_SIZE], logBuf[256];
    int commandBufLen, logBufLen;

    char logFileDirName[512];
    int logFileDirNameLength;

    // Exit on error if invalid pointer
    if(dev == NULL) {
        return -1;
    }

    // Ensure valid init mode
    if(!(mode == VN200_INIT_MODE_GPS ||
                mode == VN200_INIT_MODE_IMU ||
                mode == VN200_INIT_MODE_BOTH)) {

        logDebug("VN200Init: Mode 0x%02x not recognized.\n", mode);
        return -2;
    }

    // Ensure valid sample rate (later)

    // Initialize UART for all modes
    dev->baud = baud;
    VN200BaseInit(dev, NULL, dev->baud);

    // Initialize log file for raw and parsed data
    // Since multiple log files will be generated for the run, put them in
    // the same directory
    time_t dirtime = time(NULL);
    logFileDirNameLength = generateFilename(logFileDirName, 512, &dirtime,
            "log/SampleData/VN200", "RUN", "d");
    LogInit(&(dev->logFile), logFileDirName, "VN200", LOG_FILEEXT_LOG);
    logDebug("Logging to directory %s\n", logFileDirName);

    // If GPS enabled, init GPS log file
    if(mode & VN200_INIT_MODE_GPS) {

        // Init csv file
        LogInit(&(dev->logFileGPSParsed), logFileDirName, "VN200_GPS", LOG_FILEEXT_CSV);

        // Write header to CSV data
        logBufLen = snprintf(logBuf, 256, "gpstime,week,gpsfix,numsats,lat,lon,alt,velx,vely,velz,nacc,eacc,vacc,sacc,tacc,timestamp\n");
        LogUpdate(&(dev->logFileGPSParsed), logBuf, logBufLen);

    }

    // If IMU enabled, init IMU log file
    if(mode & VN200_INIT_MODE_IMU) {

        // Init csv file
        LogInit(&(dev->logFileIMUParsed), logFileDirName, "VN200_IMU", LOG_FILEEXT_CSV);

        // Write header to CSV data
        logBufLen = snprintf(logBuf, 256, "compx,compy,compz,accelx,accely,accelz,gyrox,gyroy,gyroz,temp,baro,timestamp\n");
        LogUpdate(&(dev->logFileIMUParsed), logBuf, logBufLen);

    }


    /**** Initialize VN200 through UART commands ****/

    // Ensure Baud rate is 115200
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

    if(mode == VN200_INIT_MODE_GPS) {

        // Enable asynchronous GPS data output
        commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,20");

    } else if(mode == VN200_INIT_MODE_IMU) {

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

    // Set sampling frequency
    dev->fs = fs;
    commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s%02d", "VNWRG,07,", dev->fs);
    VN200Command(dev, commandBuf, commandBufLen, 1);
    VN200FlushOutput(dev);
    usleep(100000);
    VN200FlushInput(dev);
    usleep(100000);

    logDebug("Done UART configuring\n\n\n\n\n\n\n\n");

    // Clear input buffer to prevent parsing "latent" data (temporary)
    VN200FlushInput(dev);

    return 0;

} // VN200Init(VN200_DEV *, int)


#if 0 // TODO REMOVE
/**** Function VN200Parse ****
 *
 * Parses data from VN200 input buffer and determine packet type. Will parse as
 * many packets as are available in the buffer
 *
 * Arguments: 
 * 	dev  - Pointer to VN200_DEV instance to parse from
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200Parse(VN200_DEV *dev) {

    // Make extra sure there is enough room in the buffer
    const int PACKET_BUF_SIZE = 1024;
    char currentPacket[PACKET_BUF_SIZE], packetID[16];

    int packetIDLength, packetIndex, logBufLen, i, rc, numParsed = 0;
    struct timespec timestamp_ts;

    // Local pointers to save for easier reference
    VN200_PACKET *packet;
    VN200_PACKET_RING_BUFFER *ringbuf;


    // Exit on error if invalid pointer
    if(dev == NULL) {
        return -1;
    }

    // Make local ringbuf pointer
    ringbuf = &(dev->ringbuf);

    // Loop through all packets in ring buffer
    for(packetIndex = ringbuf->start; packetIndex != ringbuf->end;
            packetIndex = (packetIndex + 1) % VN200_PACKET_RING_BUFFER_SIZE) {

        // Set up pointer to current packet
        packet = &(ringbuf->packets[packetIndex]);

        // If packet is incomplete, do nothing and return
        if(VN200PacketIsIncomplete(packet)) {
            return numParsed;
        }

        // Only parse if hasn't already been parsed
        if(!(packet->isParsed)) {

            int parseReturn = VN200PacketParse(ringbuf, packetIndex);

            if (parseReturn >= 0) {
                numParsed += parseReturn;
            } else {
                logDebug("VN200PacketParse failed: %d\n", parseReturn);
            }

        } // if packet not already parsed

    } // for packets in ring buffer

    return numParsed;

} // VN200Parse(VN200_DEV *)
#endif

