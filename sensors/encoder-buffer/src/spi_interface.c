

#include "spi_interface.h"

#include <wiringPiSPI.h>   // only available on Raspberry Pi 

#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*************************************************************************************************************************************************
 * Globals Variables
 * 
 * Notes:
 * Channel is the wiringPi name for the chip select (or chip enable) pin. Both channel 0 and channel 1 will be used.
 * Will add logic to allow for both later.
*************************************************************************************************************************************************/
static const int CHANNEL0 = 0;
static const int CHANNEL1 = 1;

/*************************************************************************************************************************************************
 * Function: 
 * 
 * Notes:
 * Channel is the wiringPi name for the chip select (or chip enable) pin. Both channel 0 and channel 1 will be used.
 * Will add logic to allow for both later.
*************************************************************************************************************************************************/
bool spiLogic(char* instruction)
{
    bool result;


    if( strcmp(instruction, "CLR") == 0 )          // instruction requires 1 byte
    {

    }
    else if( trcmp(instruction, "RD") == 0 )       // instruction requires 2-5 bytes
    {
          
    }
    else if( strcmp(instruction, "WR") == 0 )      // instruction requires 2-5 bytes
    {
                    
    }
    else if( strcmp(instruction, "LOAD") == 0 )    // instruction requires 1 byte
    {
                      
    }

    else                                           // generate some error code/state
    {
        printf("invalid instruction");
        result = 0;
    }
   
   return result
}

/*************************************************************************************************************************************************
 * Function: spiInterfaceSetup()
 *  
 * Description:
 *      Initializes the SPI interface; clear MDR0, MDR1, CNTR, and STR; write to MDR0 and MDR1
 * 
 * Notes:
 *      Should add logic for error handling
 *      Could change to allow for inputs into function and output from the function
*************************************************************************************************************************************************/
void spiInterfaceSetup(void)
{
    // Configure the interface, CHANNEL indicates chip select, 500000 indicates bus speed. 
    // PROBABLY NEED TO CHANGE
    int fd = wiringPiSPISetup(CHANNEL0, 500000);
    int fd = wiringPiSPISetup(CHANNEL1, 500000);

    // placeholder buffer for writing to MDR0 and MDR1
    unsigned char placeholder[] = { 0x0, 0x0 };

    // Clear (CLR) to registers MDR0, MDR1, CNTR, and STR in that order
    // Only needs 1 byte representing register and OP code
    wiringPiSPIDataRW(CHANNEL0, 0x08 , 1);
    wiringPiSPIDataRW(CHANNEL0, 0x10 , 1);
    wiringPiSPIDataRW(CHANNEL0, 0x20 , 1);
    wiringPiSPIDataRW(CHANNEL0, 0x30 , 1);

    // Write (WR) to registers MDR0 and MDR1
    // Two Bytes needed: 1st byte register & OP code, 2nd byte operating mode settings
    wiringPiSPIDataRW(CHANNEL0, placeholder , 2);    // EX: first byte: 0x88 for MDR0, second byte: 0x45
    wiringPiSPIDataRW(CHANNEL0, placeholder , 2);    // EX: first byte: 0x90 for MDR1, second byte: 0x01
}