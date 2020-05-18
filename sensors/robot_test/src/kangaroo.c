/****************************************************************************
 *
 * File:
 * 	kangaroo.c
 *
 * Description:
 * 	Hardware abstraction for interfacing with the UART Kangaroo Motion Controller
 *
 * Author:
 * 	Duncan Patel
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "config.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"

#include "kangaroo.h"

/**** Function KangarooInit ****
 *
 * Initializes a kangaroo device
 *
 * Arguments: 
 * 	dev - Pointer to Kangaroo object to initialize
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int KangarooInit(KANGAROO_DEV *dev, char *devName, char *logDirName, int baud) {

    if (dev == NULL) {
        return -1;
    }

    // Default to save log in current directory
    if (logDirName == NULL) {
        logDirName = ".";
    }

    // Default to common RPi serial device name
    if (devName == NULL) {
        devName = KANGAROO_DEVNAME;
    }

    // Initialize UART device
    dev->fd = UARTInit(devName, baud);
    if (dev->fd < 0) {
        logDebug(L_INFO, "Couldn't initialize Kangaroo motion controller UART device\n");
        return -2;
    }

    // Initialize the input and output buffers
    BufferEmpty(&(dev->inbuf));

    // Initialize log file for raw and parsed received data
    LogInit(&(dev->logFile), logDirName, "KANGAROO", LOG_FILEEXT_LOG);
    LogInit(&(dev->logFileParsed), logDirName, "ODOMETRY_K", LOG_FILEEXT_CSV);

    // Parsed CSV file header
    char *logBuf = "l_dist_mm,r_dist_mm,timestamp\n";
    LogUpdate(&(dev->logFileParsed), logBuf, strlen(logBuf));

    // Write initialization commands
    unsigned char *command;

    // Start motor channels
    command = (unsigned char *) "1,start\r\n";
    UARTWrite(dev->fd, command, strlen((char *) command));
    command = (unsigned char *) "2,start\r\n";
    UARTWrite(dev->fd, command, strlen((char *) command));
    usleep(100000);

    // Set units so commands and readback are interpreted as millimeters
    command = (unsigned char *) "1,units798mm=420lines\r\n";
    UARTWrite(dev->fd, command, strlen((char *) command));
    command = (unsigned char *) "2,units798mm=420lines\r\n";
    UARTWrite(dev->fd, command, strlen((char *) command));

    return 0;

} // KangarooInit(KANGAROO_DEV *, char *, char *, int)


/**** Function KangarooCommandSpeed ****
 *
 * Sends a speed command to both motor channels of a kangaroo device
 *
 * Arguments: 
 * 	dev    - Pointer to Kangaroo object to initialize
 * 	lSpeed - Speed to drive left motor (mm/s)
 * 	rSpeed - Speed to drive right motor (mm/s)
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int KangarooCommandSpeed(KANGAROO_DEV *dev, int lSpeed, int rSpeed) {

    int len, rc;
    unsigned char command[32];

    if (dev == NULL) {
        return -1;
    }

    len = snprintf((char *) command, 32, "%d,s%d\r\n", LEFT_MOTOR_INDEX, lSpeed);
    rc = UARTWrite(dev->fd, command, len);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to send UART speed command to motor %d: %s\n",
                LEFT_MOTOR_INDEX, strerror(errno));
        return rc;
    }

    len = snprintf((char *) command, 32, "%d,s%d\r\n", RIGHT_MOTOR_INDEX, rSpeed);
    rc = UARTWrite(dev->fd, command, len);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to send UART speed command to motor %d: %s\n",
                RIGHT_MOTOR_INDEX, strerror(errno));
        return rc;
    }

    return 0;

} // KangarooCommandSpeed(KANGAROO_DEV *, int, int)


/**** Function KangarooRequestPosition ****
 *
 * Sends a command to request the position of each motor channel
 *
 * Arguments: 
 * 	dev - Pointer to Kangaroo object to initialize
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int KangarooRequestPosition(KANGAROO_DEV *dev) {

    int rc;

    if (dev == NULL) {
        return -1;
    }

    unsigned char *command;

    command = (unsigned char *) "1,getp";
    rc = UARTWrite(dev->fd, command, strlen((char *) command));
    if (rc < 0) {
        logDebug(L_INFO, "Failed to request speed from motor 1: %s\n",
                strerror(errno));
        return -2;
    }
    command = (unsigned char *) "2,getp";
    rc = UARTWrite(dev->fd, command, strlen((char *) command));
    if (rc < 0) {
        logDebug(L_INFO, "Failed to request speed from motor 2: %s\n",
                strerror(errno));
        return -2;
    }

    return 0;

} // KangarooRequestPosition(KANGAROO_DEV *)


/**** Function KangarooPoll ****
 *
 * Polls the kangaroo serial device for input characters
 *
 * Arguments: 
 * 	dev - Pointer to Kangaroo object to initialize
 *
 * Return value:
 * 	On success, returns number of characters received
 *	On failure, returns a negative number 
 */
int KangarooPoll(KANGAROO_DEV *dev) {

    int rc, numRead, numToRead;
    unsigned char uartData[BYTE_BUFFER_MAX_LEN];

    if (dev == NULL) {
        return -1;
    }

    // Ensure length of buffer is long enough to hold more data
    if (BufferIsFull(&(dev->inbuf))) {
        logDebug(L_INFO, "Input buffer is full (%d bytes remaining). Potential loss of data.\n",
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
        logDebug(L_INFO, "%s: ioctl() failed to fetch FIONREAD\n", strerror(errno));
        // Don't return, not a fatal error
        // return -3;
    }

    logDebug(L_DEBUG, "%d bytes available from UART device...\n", ioctl_status);
#endif

    // Calculate length and pointer to proper position in array
    numToRead = BYTE_BUFFER_MAX_LEN - BufferLength(&(dev->inbuf));

    logDebug(L_VVDEBUG, "Attempting to read %d bytes from UART device...\n", numToRead);

    // Read without blocking from UART device
    numRead = UARTRead(dev->fd, uartData, numToRead);
    logDebug(L_VVDEBUG, "\tRead %d\n", numRead);

    rc = BufferAddArray(&(dev->inbuf), uartData, numRead);

    if (rc != numRead) {
        logDebug(L_INFO, "WARNING: Couldn't add all bytes read from UART to input buffer\n");
    }

    // Log newly read data to file
    LogUpdate(&(dev->logFile), (char *) uartData, numRead);

    // Return number successfully and saved to buffer (may be 0)
    return rc;

} // KangarooPoll(KANGAROO_DEV *)


/**** Function KangarooParse ****
 *
 * Sends a speed command to both motor channels of a kangaroo device
 *
 * Arguments: 
 * 	dev    - Pointer to Kangaroo object to initialize
 * 	packet - Pointer to packet object to fill with parsed data
 *
 * Return value:
 * 	On success, returns number of bytes parsed
 *	On failure, returns a negative number 
 */
int KangarooParse(KANGAROO_DEV *dev, KANGAROO_PACKET *packet) {

    /*
     * Every packet follows the form below:
     *
     *      M,Td..d\r\n
     *
     * where 
     *      M is either 1 or 2 representing the motor channel
     *      X is a single character representing the packet type
     *      d..d is multiple numeric characters representing the data for the packet
     *      \r\n is a CR-NL that marks the end of the packet
     *
     */

    int rc, packetLength;

    if (dev == NULL || packet == NULL) {
        return -1;
    }

    // Packet is invalid until parser succeeds
    packet->valid = 0;

    // Storage for the packet so that sscanf can work on a continuous buffer
    const int RAW_PACKET_MAX_LEN = 64;
    unsigned char rawPacket[RAW_PACKET_MAX_LEN];

    // Loop to parse the first valid packet it can
    int i, packetStart = 0;
    for (i = 0; i < BufferLength(&(dev->inbuf)) && !(packet->valid); i++) {
        unsigned char byte = BufferIndex(&(dev->inbuf), i);

        // Don't verify order of \r\n just in case it's inconsistent
        if (packetStart != i && (byte == '\r' || byte == '\n')) {

            // Determine packet length (clips if too long)
            packetLength = MIN(i - packetStart, RAW_PACKET_MAX_LEN - 1);

            rc = BufferCopy(&(dev->inbuf), rawPacket, packetStart, packetLength);
            if (rc != packetLength) {
                logDebug(L_INFO, "Error copying from kangaroo input buffer to parser\n");
            }

            // Insert null terminator so sscanf doesn't overrun the buffer
            rawPacket[packetLength] = '\0';

            // Parse fields from raw packet
            char rawPacketType;
            rc = sscanf((char *) rawPacket, "%1d,%c%d",
                    &(packet->channel), &rawPacketType, &(packet->data));
            if (rc != 3) {
                logDebug(L_INFO, "Error parsing fields from kangaroo packet (%d)\n", rc);
            } else {

                // If packet type is recognized, then it's a valid packet
                packet->valid = 1;
                switch (tolower(rawPacketType)) {
                    case 'e':
                        // Error packet
                        packet->type = KPT_ERROR;
                        break;
                    case 'p':
                        // Position data
                        packet->type = KPT_POS;
                        break;
                    case 's':
                        // Speed data
                        packet->type = KPT_SPEED;
                        break;
                    default:
                        // Unrecognized, packet is no longer valid
                        logDebug(L_INFO, "Unrecognized kangaroo packet type '%c' (%02x)\n", 
                                rawPacketType, rawPacketType);
                        packet->valid = 0;
                        break;
                }
            }

            // Set up for next packet if failed to parse here
            packetStart = i + 1;
        }
    }

    // Get new timestamp (not exactly accurate to when data was collected)
    getTimestamp(NULL, &(packet->timestamp));

    // Return number successfully parsed
    return packetStart;

} // KangarooParse(KANGAROO_DEV *, KANGAROO_PACKET *)


/**** Function KangarooLogOdometry ****
 *
 * Logs an odometry measurement to the kangaroo object's parse log file
 *
 * Arguments: 
 * 	dev       - Pointer to Kangaroo object to initialize
 * 	lPos      - Position of left motor to log
 * 	rPos      - Position of right motor to log
 * 	timestamp - Timestamp when data was collected
 *
 * Return value:
 * 	On success, returns number of bytes written to file
 *	On failure, returns a negative number 
 */
int KangarooLogOdometry(KANGAROO_DEV *dev, int lPos, int rPos, double timestamp) {

    int rc;
    const int BUFLEN = 128;
    unsigned char parsedBuffer[BUFLEN];

    if (dev == NULL) {
        return -1;
    }

    rc = snprintf((char *) parsedBuffer, BUFLEN, "%d,%d,%0.3f\n", lPos, rPos, timestamp);

    // Log newly parsed data to file
    LogUpdate(&(dev->logFileParsed), (char *) parsedBuffer, rc);

    return 0;

} // KangarooLogOdometry(KANGAROO_DEV *, int, int, double)


/**** Function KangarooConsume ****
 *
 * Consumes bytes in the input buffer
 *
 * Arguments: 
 * 	dev - Pointer to KANGAROO_DEV instance to modify
 * 	num - Number of bytes to consume
 *
 * Return value:
 *	On success, returns number of bytes consumed
 *	On failure, returns a negative number
 */
int KangarooConsume(KANGAROO_DEV *dev, int num) {

    // Exit on error if invalid pointer
    if (dev == NULL) {
        return -1;
    }

    num = BufferRemove(&(dev->inbuf), num);

    return num;

} // KangarooConsume(KANGAROO_DEV *, int)


/**** Function KangarooDestroy ****
 *
 * De-initializes a kangaroo object
 *
 * Arguments: 
 * 	dev - Pointer to Kangaroo object to initialize
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int KangarooDestroy(KANGAROO_DEV *dev) {

    int rc;
    unsigned char *command;

    if (dev == NULL) {
        return -1;
    }

    // Turn off motors
    command = (unsigned char *) "1,powerdown\r\n";
    rc = UARTWrite(dev->fd, command, strlen((char *) command));
    if (rc < 0) {
        logDebug(L_INFO, "Failed to powerdown kangaroo channel 1: %s\n",
                strerror(errno));
        return rc;
    }
    command = (unsigned char *) "2,powerdown\r\n";
    UARTWrite(dev->fd, command, strlen((char *) command));
    if (rc < 0) {
        logDebug(L_INFO, "Failed to powerdown kangaroo channel 1: %s\n",
                strerror(errno));
        return rc;
    }

    // Close UART file
    UARTClose(dev->fd);

    // Close log file
    LogClose(&(dev->logFile));

    return 0;

} // KangarooDestroy(KANGAROO_DEV *)

