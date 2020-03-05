/****************************************************************************
 *
 * File:
 * 	tcp.c
 *
 * Description:
 * 	Client and server interface abstraction using TCP/IP
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 03/04/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "control.h"
#include "debuglog.h"

#include "tcp.h"


/**** Function TCPInitClient ****
 *
 * Opens and initializes a TCP socket as a client. Creates a socket and 
 * establishes a connection to the server
 *
 * Arguments: 
 * 	ipAddr - String containing the IPv4 address "XXX.XXX.XXX.XXX"
 * 	port   - Integer port number to establish connection
 *
 * Return value:
 * 	On success, returns file descriptor corresponding to opened socket
 *	On failure, prints error message and returns a negative number 
 */
int TCPInitClient(char *ipAddr, int port) {

    int sock_fd, rc, socketOption;
    struct sockaddr_in socketAddress;

    // Exit on error if invalid pointer
    if (ipAddr == NULL) {
        return -1;
    }

    // Create bare socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        logDebug(L_INFO, "%s: Failed to create client sockets\n", strerror(errno));
        return rc;
    }

    // Configure socket options
    // setsockopt(sock_fd, SOL_SOCKET, SO_OPTION, &socketOption, sizeof(int));

    // Make socket nonblocking
    rc = fcntl(sock_fd, F_SETFL, O_NONBLOCK);
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to set client socket to nonblocking (not fatal)\n", strerror(errno));
        return rc;
    }

    // Configure socket address
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(port);

    // Convert IP address string into address variable
    rc = inet_pton(AF_INET, ipAddr, &(socketAddress.sin_addr));
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to convert IP address for TCP client (%s)\n", 
                ipAddr, strerror(errno));
        return rc;
    }

    // Connect to the server at ipAddr and port
    rc = connect(sock_fd, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
    if (rc == -1) {
        // May be some error conditions that we don't want to return
        if (errno == EAGAIN) {
            // Nonblocking but needs to try again
        } else {
            logDebug(L_INFO, "%s: TCP Client failed to connect to server\n", strerror(errno));
            return rc;
        }
    }

    // Return file descriptor for socket
    return sock_fd;

} // TCPInitClient(char *, int)


/**** Function TCPInitServer ****
 *
 * Opens and initializes a TCP socket for server operation. Creates a socket,
 * binds it to the port and address, and waits for a connection.
 *
 * Arguments: 
 * 	ipAddr - String containing the IPv4 address "XXX.XXX.XXX.XXX"
 * 	port   - Integer port number to listen for connection
 *
 * Return value:
 * 	On success, returns file descriptor corresponding to opened socket
 *	On failure, prints error message and returns a negative number 
 */
int TCPInitServer(char *ipAddr, int port) {

    int sock_fd, server_fd, rc;
    struct sockaddr_in socketAddress;

    // Exit on error if invalid pointer
    if (ipAddr == NULL) {
        return -1;
    }

    // Create bare socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        logDebug(L_INFO, "%s: Failed to create server socket\n", strerror(errno));
        return server_fd;
    }

    // Configure socket options
    // setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(int));

    // Make socket nonblocking
    rc = fcntl(server_fd, F_SETFL, O_NONBLOCK);
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to set server socket to nonblocking (not fatal)\n", strerror(errno));
        return rc;
    }

    // Configure socket address
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(port);

    // Convert IP address string into address variable
    rc = inet_pton(AF_INET, ipAddr, &(socketAddress.sin_addr));
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to convert IP address for TCP server (%s)\n", 
                ipAddr, strerror(errno));
        return rc;
    }

    // Bind to an incoming port to listen for client connections
    rc = bind(server_fd, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to bind server socket to address\n", strerror(errno));
        return rc;
    }

    // Listen for incoming connections (only allow one queued connection)
    rc = listen(server_fd, 1);
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to listen on server socket\n", strerror(errno));
        return rc;
    }

    // Accept an incoming connection
    sockAddressLength = sizeof(socketAddress);
    sock_fd = accept(server_fd, (struct sockaddr*)&socketAddress, &sockAddressLength);
    if (sock_fd == -1) {
        // May be some error conditions that we don't want to return
        if (errno == EAGAIN) {
            // Nonblocking but needs to try again
        } else {
            logDebug(L_INFO, "%s: TCP server failed to accept client connection\n", strerror(errno));
        }
    }

    // Return file descriptor for socket
    return sock_fd;

} // TCPInitServer(char *, int)


/**** Function TCPRead ****
 *
 * Reads from an open and initialized socket file descriptor.
 *
 * Arguments: 
 * 	sock_fd - File descriptor for open and initialized TCP socket
 *	buf     - Buffer to store data that is read
 * 	length  - Length of room left in the buffer
 *
 * Return value:
 *	Returns number of characters read (may be 0)
 *	On failure, prints error message and returns a negative number 
 */
int TCPRead(int sock_fd, unsigned char *buf, int length) {

    int numRead;

    // Exit on error if invalid pointer
    if (buf == NULL) {
        return -1;
    }

    // Ensure non-blocking read
    // Done at initialization, but placed here for assurance when misbehaving
    // if (!(fcntl(sock_fd, F_GETFL) & O_NONBLOCK)) {
    // 	fcntl(sock_fd, F_SETFL, O_NONBLOCK);
    // }

    // Attempt to receive a message from TCP socket at most length bytes (nonblocking)
    numRead = recv(sock_fd, buf, length, MSG_DONTWAIT);
    logDebug(L_VVDEBUG, "TCPRead: received %d chars\n", numRead);
    if (numRead < 0) {
        logDebug(L_INFO, "%s: TCPRead recv() failed for TCP socket\n", strerror(errno));
    }

    // Return number of bytes successfully read into buffer
    return numRead;

} // TCPRead(int, unsigned char *, int)


/**** Function TCPWrite ****
 *
 * Writes to an open and initialized file descriptor. Based on Derek Molloy's
 * RPi book
 *
 * Arguments: 
 * 	sock_fd - File descriptor for open and initialized TCP socket
 *	buf     - Buffer containing data to write
 * 	length  - Number of characters to read
 *
 * Return value:
 *	Returns number of characters written
 *	On failure, returns a negative number 
 */
int TCPWrite(int sock_fd, unsigned char *buf, int length) {

    int numWritten;

    // Exit on error if invalid pointer
    if (buf == NULL) {
        return -1;
    }

    // Attempt to write to TCP socket length bytes
    // Nonblocking and don't generate a SIGPIPE signal if the connection is broken
    numWritten = send(sock_fd, buf, length, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (numWritten < 0) {
        logDebug(L_INFO, "%s: TCPWrite send() failed for TCP socket\n", strerror(errno));
    }

    // Return number of bytes successfully read into buffer
    return numWritten;

} // TCPWrite(int, unsigned char *, int)


/**** Function TCPClose ****
 *
 * Closes the file descriptor for a TCP socket
 *
 * Arguments: 
 * 	sock_fd - File descriptor for the socket to close
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int TCPClose(int sock_fd) {

    int rc;

    rc = close(sock_fd);
    if (rc) {
        logDebug(L_INFO, "%s: TCPClose Couldn't close socket\n", strerror(errno));
    }

    return 0;

} // TCPClose(int)

