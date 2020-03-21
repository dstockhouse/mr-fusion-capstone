#include <wiringPiSPI.h>    
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int spiInterfaceLogic(char* instruction, unsigned char* register, unsigned char * data);

// channel is the wiringPi name for the chip select (or chip enable) pin.
// We will use bothe channel 0 and channel 1
// Will add logic for both later
static const int CHANNEL = 1;

int main()
{
    int fd, result;

    printf("Initializing\n");

    // Configure the interface.
    // CHANNEL insicates chip select,
    // 500000 indicates bus speed.
    fd = wiringPiSPISetup(CHANNEL, 500000);

    printf("Init result: %d\n", fd);

    // will need to change to account for the fact that some instructions only need 1 byte (no need for any data bytes)
    result = spiInterfaceLogic("LOAD", 0xE0, 0x0);      //  LOAD the CNTR
    result = spiInterfaceLogic("WR", 0x98, 0x?);        //  WR to the DTR (unsure what gets written to the DTR, documentation unclear)
    result = spiInterfaceLogic("RD", 0x68, 0x0);        //  RD to the OTR (data field is 0 since it will be cleared upon read)
    result = spiInterfaceLogic("CLR", 0x08, 0x0)        //  CLR the MDR0
}

// The string that determines which instruction must be done is closely tied with the instruction register bit pattern
// because the first w bits of the bit pattern correspond to the instruction to be done
// The data corresponds to the data that is being read from the Quad Encoder Buffer or the data that will be written to
// the Quad Encoder Buffer
unsigned char spiInterfaceLogic(char* instruction, unsigned char* intrucRegBitPattern, unsigned char* data)
{
    int result;
    unsigned char buffer[] = { &internalRegister, &data };  // do not know if will work, will test later


    if( strcmp(instruction, "CLR") == 0 )
    {
       wiringPiSPIDataRW(CHANNEL, buffer, 1);               // will always be 1 byte
    }
    else if( trcmp(instruction, "RD") == 0 )
    {
       result = wiringPiSPIDataRW(CHANNEL, buffer, 5);      // could be 2 to 5 bytes
    }
    else if( strcmp(instruction, "WR") == 0 )
    {
       wiringPiSPIDataRW(CHANNEL, buffer, 5);               // could be 2 to 5 bytes
    }
    else if( strcmp(instruction, "LOAD") == 0 )
    {
       wiringPiSPIDataRW(CHANNEL, buffer, 1);               // will always be 1 byte
    }

    else                                                    // generate some error code/state
    {
        printf("invalid instruction")       
        result = 0;
    }
   
   return result
}