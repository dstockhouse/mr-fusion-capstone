
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"

#include "utils.h"
#include "tcp.h"

#define IP_ADDR         "127.0.0.1"
#define NAVIGATION_EXE  "./navigation/navigation_main.elf"
#define CONTROL_EXE     "./sensors/robot_test/robot_test_main.elf"

int main(int argc, char **argv) {

    int rc;

    logDebug(L_INFO, "\nDEPLOYING CHILD TO RUN %s\n\n", NAVIGATION_EXE);

    pid_t fork_rc;

    fork_rc = fork();

    if (fork_rc == -1) {

        logDebug(L_INFO, "Failed to fork a child process: %s\n", strerror(errno));
        return errno;

    } else if (fork_rc == 0) {

        // We are the child, spawn application
        char *childArgs[2] = {NAVIGATION_EXE, NULL};
        rc = execv(childArgs[0], childArgs);

        // If we make it here at all the execv has failed
        if (rc == -1) {
            logDebug(L_INFO, "FAILED TO START CHILD EXECUTABLE %s: %s\n", NAVIGATION_EXE, strerror(errno));
            return -1;
        }

    } else {

        int gSock, cSock, ipSock;
        int attempt = 0, maxAttempts = 20;

        // usleep(100000);

        // Set up TCP servers
        gSock = TCPServerInit(IP_ADDR, NAVIGATION_TCP_PORT);
        TCPSetNonBlocking(gSock);

        attempt = 0;
        int unconnected = 1;
        do {
            attempt++;
            rc = TCPServerTryAccept(gSock);
            // logDebug(L_INFO, "ATTEMPT TO ACCEPT TCP CONNECTION RETURNED %d\n", rc);
            usleep(100000);
            if (rc >= 0) {
                unconnected = 0;
            }
        } while (attempt < maxAttempts && unconnected);

        if (unconnected) {

            logDebug(L_INFO, "MAX ATTEMPTS EXCEEDED; EXITING\n");

            // Kill child
            kill(fork_rc, SIGINT);

            return 1;

        } else {

            gSock = rc;

            logDebug(L_INFO, "SUCCESSFUL TCP CONNECTION TO NAVIGATION\n");
        }

        /*
        cSock = TCPClientInit();

        do {
            attempt++;
            rc = TCPClientTryConnect(cSock, IP_ADDR, CONTROL_TCP_PORT);
            usleep(100000);
        } while (attempt < maxAttempts && rc != 0 && errno == ECONNREFUSED);


        ipSock = TCPClientInit();

        do {
            attempt++;
            rc = TCPClientTryConnect(ipSock, IP_ADDR, IMAGEPROC_TCP_PORT);
            usleep(100000);
        } while (attempt < maxAttempts && rc != 0 && errno == ECONNREFUSED);
        */

        // Send it the initial conditions
        double startTime;
        struct timespec startTime_ts;
        unsigned key;

        // Initial zero time
        getTimestamp(&startTime_ts, &startTime);

        // Random key
        srand(startTime_ts.tv_sec + startTime_ts.tv_nsec);
        key = rand();

        // Send key to navigation
        char tcpBuf[16] = "init";
        // Populate timestamp and key fields
        memcpy(&(tcpBuf[4]), &startTime, 8);
        memcpy(&(tcpBuf[12]), &key, 4);

        rc = TCPWrite(gSock, (unsigned char *) tcpBuf, 16);
        if (rc < 16) {
            logDebug(L_INFO, "FAILED TO SEND INIT MESSAGE TO NAVIGATION (%d): %s\n",
                    rc, strerror(errno));
        } else {
            logDebug(L_INFO, "SENT INIT MESSAGE TO NAVIGATION: ");

/*
            int i;
            for (i = 0; i <= rc; i++) {
                if (i < 4) {
                    logDebug(L_INFO, "%c", tcpBuf[i]);
                } else {
                    logDebug(L_INFO, " %02x", tcpBuf[i]);
                }
            }
            logDebug(L_INFO, "\n");
*/
        }


        // Allow ten seconds for data collection
        sleep(10);

        // Send stop message
        rc = TCPWrite(gSock, (unsigned char *) "stop", 4);
        if (rc < 4) {
            logDebug(L_INFO, "FAILED TO SEND STOP MESSAGE TO NAVIGATION (%d): %s\n",
                    rc, strerror(errno));
        } else {
            logDebug(L_INFO, "SENT STOP MESSAGE TO NAVIGATION\n");
        }

        // Wait for child to complete
        wait(&rc);
        logDebug(L_INFO, "\nCHILD EXITED WITH CODE %d\n\n", rc);

        return rc;

    }
}

