
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

void execChild(char *executable) {

    int rc;
    char *childArgs[2] = {executable, NULL};
    rc = execv(childArgs[0], childArgs);

    // If we make it here at all the execv has failed
    if (rc == -1) {
        logDebug(L_INFO, "FAILED TO START CHILD EXECUTABLE %s: %s\n", executable, strerror(errno));
        exit(-1);
    }
}

int main(int argc, char **argv) {

    int rc;

    pid_t nav_fork_rc, control_fork_rc;

    // Fork navigation process
    logDebug(L_INFO, "\nDEPLOYING CHILD TO RUN %s\n\n", NAVIGATION_EXE);
    nav_fork_rc = fork();

    if (nav_fork_rc == -1) {

        logDebug(L_INFO, "Failed to fork a child process: %s\n", strerror(errno));
        return errno;

    } else if (nav_fork_rc == 0) {

        // We are the child, spawn application
        execChild(NAVIGATION_EXE);

    } // else


    // Fork Control Process
    logDebug(L_INFO, "\nDEPLOYING CHILD TO RUN %s\n\n", CONTROL_EXE);
    control_fork_rc = fork();

    if (control_fork_rc == -1) {

        logDebug(L_INFO, "Failed to fork a child process: %s\n", strerror(errno));
        return errno;

    } else if (control_fork_rc == 0) {

        // We are the child, spawn application
        execChild(CONTROL_EXE);

    } // else



    int nSock, cSock, ipSock;
    int attempt = 0, maxAttempts = 20;

    // usleep(100000);

    // Set up TCP servers
    nSock = TCPServerInit(IP_ADDR, NAVIGATION_TCP_PORT);
    cSock = TCPServerInit(IP_ADDR, CONTROL_TCP_PORT);
    TCPSetNonBlocking(nSock);
    TCPSetNonBlocking(cSock);

    attempt = 0;
    int nUnconnected = 1, cUnconnected = 1;
    do {
        attempt++;

        if (nUnconnected) {
            rc = TCPServerTryAccept(nSock);
            usleep(100000);
            if (rc >= 0) {
                nUnconnected = 0;
                nSock = rc;
                logDebug(L_INFO, "SUCCESSFUL TCP CONNECTION TO NAVIGATION\n");
            }
        }

        if (cUnconnected) {
            rc = TCPServerTryAccept(cSock);
            usleep(100000);
            if (rc >= 0) {
                cUnconnected = 0;
                cSock = rc;
                logDebug(L_INFO, "SUCCESSFUL TCP CONNECTION TO CONTROL\n");
            }
        }

    } while (attempt < maxAttempts && (nUnconnected || cUnconnected));

    if (nUnconnected || cUnconnected) {

        logDebug(L_INFO, "MAX TCP CONNECTION ATTEMPTS EXCEEDED; EXITING\n");

        // Kill child
        kill(nav_fork_rc, SIGINT);
        kill(control_fork_rc, SIGINT);

        return 1;

    }

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

    rc = TCPWrite(nSock, (unsigned char *) tcpBuf, 16);
    if (rc < 16) {
        logDebug(L_INFO, "FAILED TO SEND INIT MESSAGE TO NAVIGATION (%d): %s\n",
                rc, strerror(errno));
    } else {
        logDebug(L_INFO, "SENT INIT MESSAGE TO NAVIGATION: ");
    }

    rc = TCPWrite(cSock, (unsigned char *) tcpBuf, 16);
    if (rc < 16) {
        logDebug(L_INFO, "FAILED TO SEND INIT MESSAGE TO CONTROL (%d): %s\n",
                rc, strerror(errno));
    } else {
        logDebug(L_INFO, "SENT INIT MESSAGE TO CONTROL: ");
    }


    // Allow ten seconds for data collection
    sleep(10);

    // Send stop message
    rc = TCPWrite(nSock, (unsigned char *) "stop", 4);
    if (rc < 4) {
        logDebug(L_INFO, "FAILED TO SEND STOP MESSAGE TO NAVIGATION (%d): %s\n",
                rc, strerror(errno));
    } else {
        logDebug(L_INFO, "SENT STOP MESSAGE TO NAVIGATION\n");
    }

    rc = TCPWrite(cSock, (unsigned char *) "stop", 4);
    if (rc < 4) {
        logDebug(L_INFO, "FAILED TO SEND STOP MESSAGE TO CONTROL (%d): %s\n",
                rc, strerror(errno));
    } else {
        logDebug(L_INFO, "SENT STOP MESSAGE TO CONTROL\n");
    }

    // Wait for child to complete
    wait(&rc);
    logDebug(L_INFO, "\nCHILD 1 EXITED WITH CODE %d\n\n", rc);
    wait(&rc);
    logDebug(L_INFO, "\nCHILD 2 EXITED WITH CODE %d\n\n", rc);

    return rc;
}

