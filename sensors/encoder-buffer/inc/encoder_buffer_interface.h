#ifndef __ENCODER_BUFFER_INTERFACE_H
#define __ENCODER_BUFFER_INTERFACE_H

void encoderBufferRW(int intruct, int reg, unsigned char * data, int dataLength/*, static const int channel*/);
unsigned char* instructionSelect(int OPcode);
unsigned char* registerSelect(int reg);
void encoderBufferSetup(void);

#endif