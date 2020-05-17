/****************************************************************************
 *
 * File:
 *      tcp.c
 *
 * Description:
 *      Client and server interface abstraction using TCP/IP
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 03/04/2020
 *
 * Revision 0.2
 *      Last edited 03/19/2020
 *      Changed some function interfaces
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

#include "config.h"
#include "utils.h"

#include "tcp.h"


/**** Function TCPClientInit ****
 *
 * Opens and initializes a TCP socket as a client. Creates a socket but does
 * not attempt to connect to server.
 *
 * Arguments: 
 *      None
 *
 * Return value:
 *      On success, returns file descriptor corresponding to opened socket
 *      On failure, prints error message and returns a negative number 
 */
int TCPClientInit() {

    int sock_fd, rc;

    // Create bare socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        logDebug(L_INFO, "%s: Failed to create client socket\n", strerror(errno));
        return sock_fd;
    }

    // Configure socket options to allow reusing addresses
    int socketOption = 1;
    rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(int));
    if (rc != 0) {
        logDebug(L_INFO, "Unable to set socket option SO_REUSEADDR: %s\n", strerror(errno));
    }
    rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &socketOption, sizeof(int));
    if (rc != 0) {
        logDebug(L_INFO, "Unable to set socket option SO_REUSEPORT: %s\n", strerror(errno));
    }

    // Return file descriptor for socket
    return sock_fd;

} // TCPClientInit()


/**** Function TCPClientTryConnect ****
 *
 * Attempts to establish a connection to the server at ipAddr and port. If the 
 * connection cannot be made (i.e. the server is not ready), it returns 
 * immediately instead of blocking.
 *
 * Arguments: 
 *      sock_fd - File descriptor of open client socket
 *      ipAddr  - String containing the IPv4 address "XXX.XXX.XXX.XXX"
 *      port    - Integer port number to establish connection
 *
 * Return value:
 *      Returns value returned by connect, which set errno appropriately
 */
int TCPClientTryConnect(int sock_fd, char *ipAddr, int port) {

    int rc;
    struct sockaddr_in socketAddress;

    // Exit on error if invalid pointer
    if (ipAddr == NULL) {
        return -1;
    }

    // Configure socket address
    // AF_INET = Address Family InterNet
    socketAddress.sin_family = AF_INET;

    // Convert port number to network byte order
    // htons = host to network byte order (short)
    socketAddress.sin_port = htons(port);

    // Convert IP address string into address variable
    // pton = string to network address
    rc = inet_pton(AF_INET, ipAddr, &(socketAddress.sin_addr));
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to convert IP address for TCP client (%s)\n", 
                ipAddr, strerror(errno));
        return rc;
    }

    // Attempt to connect to the server at ipAddr and port
    rc = connect(sock_fd, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
    if (rc == 0) {

        // Make socket nonblocking after connecting
        // Do this here instead of at the init function so that if it doesn't try to
        // exit early during connect
        rc = fcntl(sock_fd, F_SETFL, O_NONBLOCK);
        if (rc == -1) {
            logDebug(L_INFO,
                    "%s: Failed to set client socket to nonblocking (not fatal)\n",
                    strerror(errno));
        }

    }

    // Return file descriptor for socket
    return rc;

} // TCPClientTryConnect(int, char *, int)


/**** Function TCPServerInit ****
 *
 * Opens and initializes a TCP socket for server operation. Creates a socket,
 * binds it to the port and address, and sets it to listen.
 *
 * Arguments: 
 *      ipAddr - String containing the IPv4 address "XXX.XXX.XXX.XXX"
 *      port   - Integer port number to listen for connection
 *
 * Return value:
 *      On success, returns file descriptor corresponding to opened listening socket
 *      On failure, prints error message and returns a negative number 
 */
int TCPServerInit(char *ipAddr, int port) {

    int sock_fd, rc;
    struct sockaddr_in socketAddress;

    // Exit on error if invalid pointer
    if (ipAddr == NULL) {
        return -1;
    }

    // Create bare socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        logDebug(L_INFO, "%s: Failed to create server socket\n", strerror(errno));
        return sock_fd;
    }

    // Configure socket options to allow reusing addresses
    int socketOption = 1;
    rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(int));
    if (rc != 0) {
        logDebug(L_INFO, "Unable to set socket option SO_REUSEADDR: %s\n", strerror(errno));
    }
    rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &socketOption, sizeof(int));
    if (rc != 0) {
        logDebug(L_INFO, "Unable to set socket option SO_REUSEPORT: %s\n", strerror(errno));
    }

    // Configure socket address
    // AF_INET = Address Family InterNet
    socketAddress.sin_family = AF_INET;

    // Convert port number to network byte order
    // htons = host to network byte order (short)
    socketAddress.sin_port = htons(port);

    // Convert IP address string into address variable
    // pton = string to network address
    rc = inet_pton(AF_INET, ipAddr, &(socketAddress.sin_addr));
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to convert IP address for TCP server (%s)\n", 
                ipAddr, strerror(errno));
        return rc;
    }

    // Bind to an incoming port to listen for client connections
    rc = bind(sock_fd, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to bind server socket to address\n", strerror(errno));
        return rc;
    }

    // Listen for incoming connections (only allow one queued connection)
    rc = listen(sock_fd, 1);
    if (rc == -1) {
        logDebug(L_INFO, "%s: Failed to listen on server socket\n", strerror(errno));
        return rc;
    }

    // Return file descriptor for socket
    return sock_fd;

} // TCPServerInit(char *, int)


/**** Function TCPServerTryAccept ****
 *
 * Attempts to accept a waiting connection on a listening socket. Once a
 * connection has been accepted, it closes the original socket fd.
 *
 * For server sockets, this should be called after *Init() and before *TryAccept().
 * For client sockets, this should be called after *Init() and after *TryConnect().
 *
 * Arguments: 
 *      sock_fd - File descriptor for open and listening TCP server socket
 *
 * Return value:
 *      Returns the value returned by accept(3), with errno set appropriately.
 */
int TCPServerTryAccept(int sock_fd) {

    struct sockaddr_in socketAddress;
    int newsock_fd;

    // Accept an incoming connection
    socklen_t sockAddressLength = sizeof(socketAddress);
    newsock_fd = accept(sock_fd, (struct sockaddr*)&socketAddress, &sockAddressLength);

    // If successful, close server (only one connection per server)
    if (newsock_fd != -1) {
        TCPClose(sock_fd);
    }

    // Return file descriptor for socket
    return newsock_fd;

} // TCPServerTryAccept(int)


/**** Function TCPSetNonBlocking ****
 *
 * Sets an open socket to non-blocking
 *
 * Arguments: 
 *      sock_fd - File descriptor for open TCP socket
 *
 * Return value:
 *      Returns value returned by fcntl, which set errno appropriately
 */
int TCPSetNonBlocking(int sock_fd) {

    int rc;

    // Set nonblocking and directly return fcntl return status
    rc = fcntl(sock_fd, F_SETFL, O_NONBLOCK);

    return rc;

} // TCPSetNonBlocking(int)


/**** Function TCPRead ****
 *
 * Reads from an open and initialized socket file descriptor.
 *
 * Arguments: 
 *      sock_fd - File descriptor for open and initialized TCP socket
 *      buf     - Buffer to store data that is read
 *      length  - Length of room left in the buffer
 *
 * Return value:
 *      Returns number of characters read (may be 0)
 *      On failure, prints error message and returns a negative number 
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
    //     fcntl(sock_fd, F_SETFL, O_NONBLOCK);
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
 * Sends to an open and initialized socket file descriptor.
 *
 * Arguments: 
 *      sock_fd - File descriptor for open and initialized TCP socket
 *      buf     - Buffer containing data to write
 *      length  - Number of characters to read
 *
 * Return value:
 *      Returns number of characters written
 *      On failure, returns a negative number 
 */
int TCPWrite(int sock_fd, unsigned char *buf, int length) {

    int numWritten;

    // Exit on error if invalid pointer
    if (buf == NULL) {
        return -1;
    }

    // Attempt to write to TCP socket length bytes
    // Flags:
    //      Nonblocking
    //      Don't generate a SIGPIPE signal if the connection is broken
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
 *      sock_fd - File descriptor for the socket to close
 *
 * Return value:
 *      On success, returns 0
 *      On failure, returns a negative number 
 */
int TCPClose(int sock_fd) {

    int rc;

    rc = close(sock_fd);
    if (rc == -1) {
        logDebug(L_INFO, "%s: TCPClose Couldn't close socket with fd = %d\n", strerror(errno), sock_fd);
    }

    return rc;

} // TCPClose(int)

