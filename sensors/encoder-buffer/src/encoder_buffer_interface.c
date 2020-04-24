#include "encoder_buffer_interface.h"

#include <wiringPiSPI.h>   // only available on Raspberry Pi 

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

/*********************************************************************************************************************************************
 * #define Constants
 * 
 * Description:
 *  OP Codes:  First 4 correspond to instruction OP codes
 *  Registers: The last 6 correspond to internal registers
 * 
 * Notes:
 *  A possible change would be for these to be bit masks.
 *  
*********************************************************************************************************************************************/
#define CLR 0
#define RD  1
#define WR  2
#define LD  3

#define MDR0 0
#define MDR1 1
#define DTR  2
#define CNTR 3
#define OTR  4
#define STR  5

/*********************************************************************************************************************************************
 * Globals Variables
 * 
 * Notes:
 * Possibly change the channel global variables to #define constants.
*********************************************************************************************************************************************/
static const int CHANNEL0 = 0;
static const int CHANNEL1 = 1;

/*********************************************************************************************************************************************
 * Function: encoderBufferRW
 * 
 * Description:
 *  Will determine the bit pattern corresponding to the instruction and register recieved. Then transmit the 1 or more byte to the encoder
 *  buffer ( Byte 1: OP code + register; Bytes 2-5: Data)
 * Notes:
 * 
*********************************************************************************************************************************************/
void encoderBufferRW(int intruct, int reg, unsigned char * data, int dataLength/*, static const int channel*/)
{
    unsigned char* firstByte = 0xFF; //the first byte sent corresponds to the OP code and register

    unsigned char* instructBitMask = instructionSelect(instruct);
    //error checking and logging for invalid instruction OP code

    unsigned char* regBitMask = registerSelect(reg)
    // error checking and logging for invalid register name

    /************************************/
    /* bit masking logic goes here */
    /************************************/

    unsigned char* buffer = 0x00; // final value to be passed to encoder buffer; should be array
    
    wiringPiSPIDataRW(CHANNEL0, buffer , 1);

}


/*********************************************************************************************************************************************
 * Function: instructionSelect()
 * 
 * Description:
 *  Input:  Instruction number corresponding to one of the #define opcodes
 *  Output: Some bit mask that would correspond to the OP code of the instruction
 * 
 * Notes:
 * 
*********************************************************************************************************************************************/
unsigned char* instructionSelect(int OPcode)
{
    unsigned char* bitMask;
    switch (OPcode)
    {
        case CLR:           // Clear
            bitMask = 0x3F; // 0011 1111
            break;
        case RD:            // Read
            bitMask = 0x7F; // 0111 1111
            break;
        case WR:            // Write
            bitMask = 0xBF; // 1011 1111
            break;
        case LD:            // Load
            bitMask = 0xFF; // 1111 1111
            break;
        default:            // Error State
            bitMask = 0x00; 
            break;
    }
    return bitMask;
}

/*********************************************************************************************************************************************
 * Function: registerSelect()
 * 
 * Description:
 *  Input: Register number corresponding to one of the #define registers
 *  Output: Some bit mask that would correspond to the register
 * 
 * Notes:
 * 
*********************************************************************************************************************************************/
unsigned char* registerSelect(int reg)
{
    unsigned char* bitMask;
    switch (reg)
    {
        case MDR0:          // mode register 0
            bitMask = 0xCF  // 1100 1111
            break;
        case MDR1:          // mode register 1
            bitMask = 0xDF  // 1101 1111
            break;
        case DTR:           // 
            bitMask = 0xD7  // 1101 0111
            break;
        case CNTR:          // count register
            bitMask = 0xE7  // 1110 0111
            break;
        case OTR:           // 
            bitMask = 0xEF  // 1110 1111
            break;
        case STR:           // status register
            bitMask = 0xF7  // 1111 0111
            break;
        default:            // error state
            bitMask = 0x00;
            break;
    }
    return bitMask;
}

/*********************************************************************************************************************************************
 * Function: spiInterfaceSetup()
 *  
 * Description:
 *      Initializes the SPI interface; clear MDR0, MDR1, CNTR, and STR; write to MDR0 and MDR1
 * 
 * Notes:
 *      Should add logic for error handling
 *      Could change to allow for inputs into function and output from the function
 *      Should change so that it calls encoderBufferRW() function.
*********************************************************************************************************************************************/
void encoderBufferSetup(void)
{
    // Configure the interface, CHANNEL indicates chip select, 500000 indicates bus speed. 
    // PROBABLY NEED TO CHANGE
    int fd = wiringPiSPISetup(CHANNEL0, 500000);
    int fd = wiringPiSPISetup(CHANNEL1, 500000);

    // Clear (CLR) to registers MDR0, MDR1, CNTR, and STR in that order
    // Only needs 1 byte representing register and OP code
    wiringPiSPIDataRW(CHANNEL0, 0x0F , 1);  // 0000 1111
    wiringPiSPIDataRW(CHANNEL0, 0x17 , 1);  // 0001 0111
    wiringPiSPIDataRW(CHANNEL0, 0x27 , 1);  // 0010 0111
    wiringPiSPIDataRW(CHANNEL0, 0x37 , 1);  // 0011 0111

    // placeholder buffer for writing to MDR0 and MDR1
    unsigned char buffer1[] = { 0x8F, 0x45 }; // 1000 1111, 0100 0101; example, change needed
    unsigned char buffer2[] = { 0x97, 0x09 }; // 1001 0111, 0000 1001; example, change needed

    // Write (WR) to registers MDR0 and MDR1
    // Two Bytes needed: 1st byte register & OP code, 2nd byte operating mode settings
    wiringPiSPIDataRW(CHANNEL0, buffer, 2);    
    wiringPiSPIDataRW(CHANNEL0, buffer, 2);    
}