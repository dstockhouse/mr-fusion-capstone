#ifndef __SPI_INTERFACE_H
#define __SPI_INTERFACE_H

void encoderBufferRW(int intruct, int reg, unsigned char * data, int dataLength/*, static const int channel*/);
void spiInterfaceSetup(void);

#endif