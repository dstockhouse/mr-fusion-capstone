#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

#include "tcp.h"

#define IP_ADDR     "127.0.0.1"
#define SERVER_PORT 42020
#define CLIENT_PORT 42040

// Name of test context
Describe(TCPInterface);

// Execute in the context immediately before each "Ensure" test
BeforeEach(TCPInterface) {
}

// Execute after each test
AfterEach(TCPInterface) {
}


/**** Start test suite ****/

Ensure(TCPInterface, successfully_initializes_loopback) {
    int client_fd, server_fd, rc;

    client_fd = TCPInitClient(IP_ADDR, PORT);
    server_fd = TCPInitServer(IP_ADDR, PORT);

    assert_that(client_fd, is_not_equal_to(-1));
    assert_that(server_fd, is_not_equal_to(-1));

    rc = TCPClose(client_fd);
    assert_that(rc, is_equal_to(0));
    rc = TCPClose(server_fd);
    assert_that(rc, is_equal_to(0));
}

