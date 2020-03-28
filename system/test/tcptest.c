/****************************************************************************
 *
 * File:
 *      tcptest.c
 *
 * Description:
 *      CGreen test suite for the TCP interface abstraction module (tcp.c)
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 03/19/2020
 *
 ***************************************************************************/

#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include "config.h"

#include "tcp.h"

#define IP_ADDR     "127.0.0.1"
#define SERVER_PORT NAVIGATION_TCP_PORT
#define CLIENT_PORT CONTROL_TCP_PORT

int client_fd, server_fd, rc;

// Name of test context
Describe(TCPInterface);

// Execute in the context immediately before each "Ensure" test
BeforeEach(TCPInterface) {
}

// Execute after each test
AfterEach(TCPInterface) {

    // Close sockets
    rc = TCPClose(client_fd);
    assert_that(rc, is_equal_to(0));
    rc = TCPClose(server_fd);
    assert_that(rc, is_equal_to(0));

}


/**** Function setup_sockets
 *
 * This function is the simplest example of how to properly initialize a socket
 * connection, without taking advantage of the nonblocking ability. It shows the
 * correct order to initiate API calls.
 *
 ****/
void setup_sockets(int perform_asserts) {

    // Init both
    client_fd = TCPClientInit();
    server_fd = TCPServerInit(IP_ADDR, SERVER_PORT);
    if (perform_asserts) {
        assert_that(client_fd, is_not_equal_to(-1));
        assert_that(server_fd, is_not_equal_to(-1));
    }

    // Connect client to server
    rc = TCPClientTryConnect(client_fd, IP_ADDR, SERVER_PORT);
    if (perform_asserts) {
        assert_that(rc, is_not_equal_to(-1));
    }

    // Set server nonblocking for accept
    rc = TCPSetNonBlocking(server_fd);
    if (perform_asserts) {
        assert_that(rc, is_not_equal_to(-1));
    }

    // Accept client from server
    rc = TCPServerTryAccept(server_fd);
    if (perform_asserts) {
        assert_that(rc, is_not_equal_to(-1));
    }
    // Old server FD already closed, save new one
    server_fd = rc;

    // Set client nonblocking for future calls to read
    rc = TCPSetNonBlocking(client_fd);
    if (perform_asserts) {
        assert_that(rc, is_not_equal_to(-1));
    }

}


/**** Start test suite ****/

Ensure(TCPInterface, successfully_initializes_loopback) {

    client_fd = TCPClientInit();
    server_fd = TCPServerInit(IP_ADDR, SERVER_PORT);

    assert_that(client_fd, is_not_equal_to(-1));
    assert_that(server_fd, is_not_equal_to(-1));

}

Ensure(TCPInterface, client_fails_with_async_connect) {

    client_fd = TCPClientInit();
    assert_that(client_fd, is_not_equal_to(-1));

    // Attempt to connect to a nonexistent server
    rc = TCPClientTryConnect(client_fd, IP_ADDR, SERVER_PORT);
    assert_that(rc, is_equal_to(-1));
    assert_that(errno, is_equal_to(ECONNREFUSED));

    // Open to be closed
    server_fd = TCPServerInit(IP_ADDR, SERVER_PORT);

}

Ensure(TCPInterface, server_fails_with_async_accept) {

    server_fd = TCPServerInit(IP_ADDR, SERVER_PORT);
    assert_that(server_fd, is_not_equal_to(-1));

    // Set nonblocking for attempted accept
    rc = TCPSetNonBlocking(server_fd);
    assert_that(rc, is_not_equal_to(-1));

    // Attempt to accept a nonexistent client
    rc = TCPServerTryAccept(server_fd);
    assert_that(rc, is_equal_to(-1));
    assert_that(errno, is_equal_to(EAGAIN));

    // Open to be closed
    client_fd = TCPClientInit();

}

Ensure(TCPInterface, client_server_connect_loopback) {

    // Use function above to set up proper connection, with assertions
    setup_sockets(1);

}

Ensure(TCPInterface, connect_out_of_order_client_first) {

    // Setup client
    client_fd = TCPClientInit();
    assert_that(client_fd, is_not_equal_to(-1));

    // Attempt to connect to a nonexistent server
    rc = TCPClientTryConnect(client_fd, IP_ADDR, SERVER_PORT);
    assert_that(rc, is_equal_to(-1));
    assert_that(errno, is_equal_to(ECONNREFUSED));

    // Setup server
    server_fd = TCPServerInit(IP_ADDR, SERVER_PORT);
    assert_that(server_fd, is_not_equal_to(-1));

    // Connect client to server
    rc = TCPClientTryConnect(client_fd, IP_ADDR, SERVER_PORT);
    assert_that(rc, is_not_equal_to(-1));

    // Set server nonblocking for accept
    rc = TCPSetNonBlocking(server_fd);
    assert_that(rc, is_not_equal_to(-1));

    // Accept client from server
    rc = TCPServerTryAccept(server_fd);
    assert_that(rc, is_not_equal_to(-1));

    // Old FD already closed, save new one
    server_fd = rc;

}

Ensure(TCPInterface, connect_out_of_order_server_first) {

    // Setup server
    server_fd = TCPServerInit(IP_ADDR, SERVER_PORT);
    assert_that(server_fd, is_not_equal_to(-1));

    // Set server nonblocking for accept
    rc = TCPSetNonBlocking(server_fd);
    assert_that(rc, is_not_equal_to(-1));

    // Attempt to accept nonexistent client from server
    rc = TCPServerTryAccept(server_fd);
    assert_that(rc, is_equal_to(-1));
    assert_that(errno, is_equal_to(EAGAIN));

    // Setup client
    client_fd = TCPClientInit();
    assert_that(client_fd, is_not_equal_to(-1));

    // Connect client to server
    rc = TCPClientTryConnect(client_fd, IP_ADDR, SERVER_PORT);
    assert_that(rc, is_not_equal_to(-1));

    // Accept client from server
    rc = TCPServerTryAccept(server_fd);
    assert_that(rc, is_not_equal_to(-1));

    // Old FD already closed, save new one
    server_fd = rc;

}

Ensure(TCPInterface, send_message_client_to_server) {

    // Use function above to set up proper connection, without assertions
    setup_sockets(0);

    unsigned char outmsg[256], inmsg[256];
    strcpy(outmsg, "Howdy server");
    int msglen = strlen(outmsg);

    // Send message
    rc = TCPWrite(client_fd, outmsg, msglen);
    assert_that(rc, is_equal_to(msglen));

    // Pause 10ms
    usleep(10000);

    // Check if input data is available
    struct pollfd fds;
    fds.fd = server_fd;
    fds.events = POLLIN;
    rc = poll(&fds, 1, 1);
    assert_that(rc, is_equal_to(1));
    assert_that(fds.revents & POLLIN, is_true);

    // Receive message
    rc = TCPRead(server_fd, inmsg, 256);
    assert_that(rc, is_equal_to(msglen));
    assert_that(outmsg[0], is_equal_to(inmsg[0]));
    assert_that(outmsg[1], is_equal_to(inmsg[1]));
    assert_that(outmsg[rc-2], is_equal_to(inmsg[rc-2]));
    assert_that(outmsg[rc-1], is_equal_to(inmsg[rc-1]));

}

