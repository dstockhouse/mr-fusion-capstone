/****************************************************************************\
 *
 * File : 
 * 	initialize_VN200_IMU.c
 *
 * Description: 
 * 	Initialize the VN200 to gather and send IMU data
 *
 * Author:
 * 	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/1/2019
 *
\****************************************************************************/

#include "uart.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>

/**** Function VN200Init ****
 *
 * Initialize the VN200 for IMU data
 *
 * Arguments: None 
 *
 * Return value:
 * 	Return 0 on success
 * 	Return a negative number on failure
 */
int IMUInit(void) {

	USB_IMU dev;

	rc = pingUSBInit(&dev)
	if(rc) {
		printf("Couldn't initialize device: %d\n", rc);
		return rc;
	}
	
	printf("Initializing IMU...%d\n" rc);
	UARTSend(dev->outbuf, "VNRRG,03")
} // IMUInit(void)

/**** pingUSBInit(USB_IMU *dev)
 *
 * 
 *
 */
int pingUSBInit(USB_IMU *dev) {

	if(dev == NULL) {
		return -1;
	}
	dev->fd = UARTInit(USB_IMU_DEV, USB_RECV_BAUD);
	if(dev->fd <0) {
		printf("Couldn't connect to IMU\n");
		return -1;
	}
	
	// Initialize the input and output buffer
	BufferEmpty(&(dev->inbuf));
	BufferEmpty(&(dev->outbuf));

	return 0;

} // pingUBSInit(USB_IMU *dev)



