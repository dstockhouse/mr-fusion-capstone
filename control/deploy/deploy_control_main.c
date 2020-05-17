
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"

#include "utils.h"
#include "tcp.h"

#define IP_ADDR     "127.0.0.1"
#define CHILD_EXE   "./control_main.elf"

int main(int argc, char **argv) {

    int rc;

    logDebug(L_INFO, "\nDEPLOYING CHILD TO RUN %s\n\n", CHILD_EXE);

    pid_t fork_rc;

    fork_rc = fork();

    if (fork_rc == -1) {

        logDebug(L_INFO, "Failed to fork a child process: %s\n", strerror(errno));
        return errno;

    } else if (fork_rc == 0) {

        // We are the child, spawn application
        char *childArgs[2] = {CHILD_EXE, NULL};
        rc = execv(childArgs[0], childArgs);

        // If we make it here at all the execv has failed
        if (rc == -1) {
            logDebug(L_INFO, "FAILED TO START CHILD EXECUTABLE %s: %s\n", CHILD_EXE, strerror(errno));
            return -1;
        }

    } else {

        int gSock, nSock;
        int attempt = 0, maxAttempts = 10;

        // usleep(100000);

        // Set up TCP servers
        gSock = TCPServerInit(IP_ADDR, CONTROL_TCP_PORT);
        TCPSetNonBlocking(gSock);

        attempt = 0;
        do {
            attempt++;
            rc = TCPServerTryAccept(gSock);
            usleep(100000);
        } while (attempt < maxAttempts && rc != 0 && errno == EBUSY);


        nSock = TCPServerInit(IP_ADDR, CONTROL_TCP_PORT);
        TCPSetNonBlocking(nSock);

        do {
            attempt++;
            rc = TCPServerTryAccept(nSock);
            usleep(100000);
        } while (attempt < maxAttempts && rc != 0 && errno == EBUSY);


        // Wait for child to complete
        wait(&rc);
        logDebug(L_INFO, "\nCHILD EXITED WITH CODE %d\n\n", rc);

        return rc;

    }
}

