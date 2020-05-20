
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
#define CONTROL_EXE     "./control/control_main.elf"

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

    int i, rc;

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

    // Random key
    srand(startTime_ts.tv_sec + startTime_ts.tv_nsec);
    key = rand();

    // Initial zero time
    getTimestamp(&startTime_ts, &startTime);

    // Send key to subsystems
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

    // Delay to let subsystems start operation
    sleep(2);

    // Non blocking terminal input
    setStdinNoncanonical(1);

    int loopContinue = 1;
    double speed = 0.0, rotation = 0.0;
    int escapeMode = 0;
    double endTime;
    while (loopContinue) {

        // Get input from user nonblocking
        char input[16];
        rc = read(STDIN_FILENO, input, 16);

        // speed = 0.0;
        // rotation = 0.0;

        for (i = 0; i < rc; i++) {
            switch (input[i]) {

                // Escape code
                case '\033':
                    escapeMode = 1;
                    break;

                    // Linear speed control
                case 'A': // Up arrow
                    if (escapeMode) {
                        speed += 0.2;
                        escapeMode = 0;
                    }
                    break;
                case 'B': // Down arrow
                    if (escapeMode) {
                        speed -= 0.2;
                        escapeMode = 0;
                    }
                    break;

                    // Angular speed control
                case 'C': // Left arrow
                    if (escapeMode) {
                        rotation += 0.2;
                        escapeMode = 0;
                    }
                    break;
                case 'D': // Right arrow
                    if (escapeMode) {
                        rotation -= 0.2;
                        escapeMode = 0;
                    }
                    break;

                case ' ':
                    speed = 0;
                    rotation = 0;
                    break;

                case 'q':
                    // Exit loop
                    loopContinue = 0;
                    break;

                    // Ignore parts of special characters
                case '[':
                case '~':
                    // No input
                    break;

                default:
                    strncpy(tcpBuf, "ctlx", 4);
                    speed = 0;
                    rotation = 0;
                    escapeMode = 0;
                    break;
            }
        }

        if (speed > 0.8) {
            speed = 0.8;
        } else if (speed < -0.8) {
            speed = -0.8;
        }
        if (rotation > 0.8) {
            rotation = 0.8;
        } else if (rotation < -0.8) {
            rotation = -0.8;
        }

        // Send speed and rotation commands
        strncpy(tcpBuf, "ctls", 4);
        memcpy((unsigned char *) &(tcpBuf[4]), &speed, 8);
        rc = TCPWrite(cSock, tcpBuf, 12);
        if (rc < 12) {
            logDebug(L_INFO, "FAILED TO SEND SPEED MESSAGE TO CONTROL (%d): %s\n",
                    rc, strerror(errno));
        }
        strncpy(tcpBuf, "ctlr", 4);
        memcpy((unsigned char *) &(tcpBuf[4]), &rotation, 8);
        rc = TCPWrite(cSock, tcpBuf, 12);
        if (rc < 12) {
            logDebug(L_INFO, "FAILED TO SEND ROTATION MESSAGE TO CONTROL (%d): %s\n",
                    rc, strerror(errno));
        }

        logDebug(L_INFO, "  S: %.3f  R: %.3f   \r", speed, rotation);

        // Delay for the loop to run more slowly
        struct timespec delaytime;
        delaytime.tv_sec = 0;
        delaytime.tv_nsec = 20000000; // 20 ms
        nanosleep(&delaytime, NULL);

        // Print data capture duration
        getTimestamp(NULL, &endTime);
        logDebug(L_INFO, "%.1lf\r", endTime);
    }

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

    setStdinNoncanonical(0);

    // Wait for child to complete
    pid_t childPid;
    childPid = wait(&rc);
    logDebug(L_INFO, "\n%s EXITED WITH CODE %d\n\n", (childPid==nav_fork_rc)?"NAVIGATION":"CONTROL", rc);
    childPid = wait(&rc);
    logDebug(L_INFO, "\n%s EXITED WITH CODE %d\n\n", (childPid==nav_fork_rc)?"NAVIGATION":"CONTROL", rc);

    return rc;
}

