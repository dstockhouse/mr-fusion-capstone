#ifndef __KANGAROO_INTERFACE_H
#define __KANGAROO_INTERFACE_H

void kangarooInterfaceSetup(void);
void kangarooSpeedControl(unsigned char* buffer, int fd);
unsigned char* kangarooSpeedReadback(unsigned char* buffer, int fd);
unsigned char* kangarooReadbackError(unsigned char* readback);

#endif