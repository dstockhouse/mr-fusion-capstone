/****************************************************************************
 *
 * File:
 *      encoder_run.c
 *
 * Description:
 * 	    Software thread for reading from hardware encoder buffer over SPI
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 05/19/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "utils.h"

#include "control.h"

int encoder_continueRunning = 1;

int encoder_run(CONTROL_PARAMS *control) {

    KANGAROO_PACKET packet;
    int rc;

    // Local pointer to Kangaroo device
    KANGAROO_DEV *dev = &(control->kangaroo);

    int lPos, rPos, newRawOdometry = 0, numConsumed;
    double lTimestamp, rTimestamp;

    printf("  Starting encoder thread\n");

    while (encoder_continueRunning) {

        rc = KangarooPoll(dev);
        if (rc < 0) {
            logDebug(L_INFO, "Kangaroo Read thread failed to poll UART Device: %s\n",
                    strerror(errno));
        }

        do {

            numConsumed = 0;

            rc = KangarooParse(dev, &packet);
            if (rc < 0) {
                logDebug(L_INFO, "Kangaroo Read thread failed to parse bytes from packet: %s\n",
                        strerror(errno));
            } else {

                rc = KangarooConsume(dev, rc);
                if (rc < 0) {
                    logDebug(L_INFO, "Failed to consume bytes\n");
                } else {
                    numConsumed = rc;
                }

                if (packet.valid && packet.type == KPT_POS) {
                    if (packet.channel == LEFT_MOTOR_INDEX) {
                        lPos = packet.data;
                        lTimestamp = packet.timestamp - control->startTime;
                        newRawOdometry = 1;
                    } else if (packet.channel == RIGHT_MOTOR_INDEX) {
                        rPos = packet.data;
                        rTimestamp = packet.timestamp - control->startTime;
                        newRawOdometry = 1;
                    }
                } else {
                    // Unexpected packet type
                }

                // If packets have a similar timestamp, then assume they are the same
                if (newRawOdometry) {
                    newRawOdometry = 0;

                    // If less than 50 ms different, assume they are the same reading
                    if (fabs(lTimestamp - rTimestamp) < 0.05) {

                        // Log to file
                        rc = KangarooLogOdometry(dev, lPos, rPos, MIN(lTimestamp, rTimestamp));
                        if (rc < 0) {
                            logDebug(L_INFO, "Unable to log odometry data to file\n");
                        }
                    }
                }

            } // if/else parse succeeded

        } while (numConsumed > 0);

    } // while (encoder_continueRunning)

    printf("  Ending encoder thread\n");

	return 0;
}

