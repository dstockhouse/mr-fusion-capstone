/****************************************************************************
 *
 * File:
 * 	uart.c
 *
 * Description:
 * 	Interfaces with a USB serial device
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
 * Revision 0.2
 * 	Included integration with logger
 * 	Last edited 2/20/2019
 *
 * Revision 0.3
 * 	Split UART and ADS-B specific receiver into separate modules
 * 	Last edited 4/01/2019
 *
 * Revision 0.4
 * 	Added UARTWrite function
 * 	Last edited 4/20/2019
 *
 * Revision 0.5
 * 	Added access() for permission checking
 * 	Last edited 5/08/2019
 *
 ***************************************************************************/

#include "config.h"
#include "debuglog.h"

#include "uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


/**** Function UARTInit ****
 *
 * Opens and initializes a UART device. Based mostly on Derek Molloy's RPi book
 *
 * Arguments: 
 * 	devName - String name of the file the UART device is at
 * 	baud    - Requested baud rate (for now must be either 115200 or 57600)
 *
 * Return value:
 * 	On success, returns file descriptor corresponding to UART device
 *	On failure, prints error message and returns a negative number 
 */
int UARTInit(char *devName, int baud) {

    int uart_fd, rc;

    // Exit on error if invalid pointer
    if (devName == NULL) {
        return -1;
    }

    // Ensure user has permissions to access UART file
    rc = access(devName, R_OK | W_OK);
    if (rc == -1) {
        logDebug(L_INFO, "%s: Cannot access UART device (try as sudo?)\n", strerror(errno));
        return rc;
    }

    // Open UART file
    // uart_fd = open(devName, O_RDWR | O_NOCTTY | O_NDELAY);
    uart_fd = open(devName, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        logDebug(L_INFO, "%s: UARTInit open() failed for UART device\n", strerror(errno));
        return uart_fd;
    }

    // Set baud rate
    UARTSetBaud(uart_fd, baud);

    // Return file descriptor for UART
    return uart_fd;

} // UARTInit(char *, int)


/**** Function UARTInitReadOnly ****
 *
 * Opens and initializes a UART device. Based mostly on Derek Molloy's RPi book
 *
 * Arguments: 
 * 	devName - String name of the file the UART device is at
 * 	baud    - Requested baud rate (for now must be either 115200 or 57600)
 *
 * Return value:
 * 	On success, returns file descriptor corresponding to UART device
 *	On failure, prints error message and returns a negative number 
 */
int UARTInitReadOnly(char *devName, int baud) {

    int uart_fd, rc;

    // Exit on error if invalid pointer
    if (devName == NULL) {
        return -1;
    }

    // Ensure user has permissions to access UART file
    rc = access(devName, R_OK);
    if (rc == -1) {
        logDebug(L_INFO, "%s: Cannot access UART device (try as sudo?)\n", strerror(errno));
        return rc;
    }

    // Open UART file
    uart_fd = open(devName, O_RDONLY | O_NOCTTY);
    if (uart_fd < 0) {
        logDebug(L_INFO, "%s: UARTInit open() failed for UART device\n", strerror(errno));
        return uart_fd;
    }

    // Set baud rate
    UARTSetBaud(uart_fd, baud);

    // Return file descriptor for UART
    return uart_fd;

} // UARTInitReadOnly(char *, int)


int UARTSetBaud(int fd, int baud) {

    struct termios uartOptions;
    int rc;

    // Get existing device attributes
    rc = tcgetattr(fd, &uartOptions);
    if (rc == -1) {
        logDebug(L_INFO, "%s: UARTSetBaud tcgetattr() failed for UART device\n", strerror(errno));
        return rc;
    }

    // Set new termios control options
    uartOptions.c_cflag &= ~(CSIZE | PARENB | CSTOPB | CRTSCTS);
    uartOptions.c_cflag |= CS8 | CREAD | CLOCAL;

    // Set input options
    uartOptions.c_iflag &= ~(ICRNL);
    uartOptions.c_iflag |= IGNPAR | IXON | IXOFF;

    // Set output options
    uartOptions.c_oflag &= ~(OPOST | ONLCR);
    uartOptions.c_oflag |= OCRNL;

    // Set local options
    uartOptions.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE);
    uartOptions.c_lflag |= 0;

    // Set additional options
    uartOptions.c_cc[VTIME] = 1;
    uartOptions.c_cc[VMIN] = 0;

    // Set baud rate based on input, bare minimum so far supported
    switch(baud) {
        case 115200:
            uartOptions.c_cflag |= B115200;
            cfsetispeed(&uartOptions, B115200);
            cfsetospeed(&uartOptions, B115200);
            break;
        case 57600:
            uartOptions.c_cflag |= B57600;
            cfsetispeed(&uartOptions, B57600);
            cfsetospeed(&uartOptions, B57600);
            break;
        case 9600:
            uartOptions.c_cflag |= B9600;
            cfsetispeed(&uartOptions, B9600);
            cfsetospeed(&uartOptions, B9600);
            break;
        default:
            logDebug(L_INFO, "Unexpected baud rate: %d. Exiting\n", baud);
            close(fd);
            return -3;
            break;
    }

    // Push changed options to device (after flushing input and output)
    rc = tcflush(fd, TCIOFLUSH);
    if (rc == -1) {
        logDebug(L_INFO, "%s: UARTSetBaud tcflush() failed for UART device\n", strerror(errno));
        return rc;
    }

    rc = tcsetattr(fd, TCSANOW, &uartOptions);
    if (rc == -1) {
        logDebug(L_INFO, "%s: UARTSetBaud tcsetattr() failed for UART device\n", strerror(errno));
        return rc;
    }

    return 0;
}

/**** Function UARTRead ****
 *
 * Reads from an open and initialized file descriptor. Based on Derek Molloy's
 * RPi book
 *
 * Arguments: 
 * 	uart_fd - File descriptor for open and initialized UART device
 *	buf     - Buffer to store data that is read
 * 	length  - Length of room left in the buffer
 *
 * Return value:
 *	Returns number of characters read (may be 0)
 *	On failure, prints error message and returns a negative number 
 */
int UARTRead(int uart_fd, unsigned char *buf, int length) {

    int numRead;

    // Exit on error if invalid pointer
    if (buf == NULL) {
        return -1;
    }

    // Ensure non-blocking read
    // if (!(fcntl(uart_fd, F_GETFL) & O_NONBLOCK)) {
    // 	fcntl(uart_fd, F_SETFL, O_NONBLOCK);
    // }

    // Attempt to read from UART device at most length bytes
    numRead = read(uart_fd, buf, length);
    // printf("UARTRead: read %d chars\n", numRead);
    if (numRead < 0) {
        logDebug(L_INFO, "%s: UARTread read() failed for UART device\n", strerror(errno));
    }

    // Return number of bytes successfully read into buffer
    return numRead;

} // UARTRead(int, unsigned char *, int)


/**** Function UARTWrite ****
 *
 * Writes to an open and initialized file descriptor. Based on Derek Molloy's
 * RPi book
 *
 * Arguments: 
 * 	uart_fd - File descriptor for open and initialized UART device
 *	buf     - Buffer containing data to write
 * 	length  - Number of characters to read
 *
 * Return value:
 *	Returns number of characters written
 *	On failure, prints error message and returns a negative number 
 */
int UARTWrite(int uart_fd, unsigned char *buf, int length) {

    int numWritten;

    // Exit on error if invalid pointer
    if (buf == NULL) {
        return -1;
    }

    // Attempt to write to UART device length bytes
    numWritten = write(uart_fd, buf, length);
    if (numWritten < 0) {
        logDebug(L_INFO, "%s: UARTWrite write() failed for UART device\n", strerror(errno));
    }

    // Return number of bytes successfully read into buffer
    return numWritten;

} // UARTWrite(int, unsigned char *, int)


/**** Function UARTClose ****
 *
 * Closes the file descriptor for a UART device
 *
 * Arguments: 
 * 	uart_fd - File descriptor for the device to close
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int UARTClose(int uart_fd) {

    int rc;

    rc = close(uart_fd);
    if (rc == -1) {
        logDebug(L_INFO, "%s: UARTClose Couldn't close UART file\n", strerror(errno));
    }

    return rc;

} // UARTClose(int)

