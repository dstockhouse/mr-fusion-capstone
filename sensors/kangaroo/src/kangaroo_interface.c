#include "uart.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*********************************************************************************************************************************************
 * Function: kangarooInterfaceSetup()
 * 
 * Description:
 *  Will initialize the UART interface, and initialize the Kangaroo.
 * Notes:
 *  Not sure if "/dev/ttyS0" is the correct device name.
 *  Need to change the hardcoded UARTWrite function.
 *  Need to add error checking between charWritten and the size of the buffer sent.
*********************************************************************************************************************************************/
void kangarooInterfaceSetup(void)
{
    int fd = UARTInit("/dev/ttyS0", 9600);
    int charWritten = UARTWrite(fd, "1,start", 8);
    int charWritten = UARTWrite(fd, "2,start", 8);
    int charWritten = UARTWrite(fd, "1,units 100 cm = 2048 lines", 28);
    int charWritten = UARTWrite(fd, "2,units 100 cm = 2048 lines", 28);
}

/*********************************************************************************************************************************************
 * Function: kangarooSpeedControl()
 * 
 * Description:
 *  Inputs: Character buffer for channel number, a comma, the letter s, then the speed number
 * Notes:
 *  Possibly needs logic to convert from a floating point number to an unsigned character pointer.
 *  Possibly remake this into a generic write function for all times writing to Kangaroo is necessary.
*********************************************************************************************************************************************/
void kangarooSpeedControl(unsigned char* buffer, int fd)
{
    
    int charWritten = UARTWrite(fd, buffer, strlen(buffer));
}
/*********************************************************************************************************************************************
 * Function: kangarooSpeedReadback()
 * 
 * Description:
 *  Inputs: Character Buffer for the channel and the speed readback command, fd is the file descriptor for the UART
 *  Output: The character string of the current speed of the wheel
 * Notes:
 * 
*********************************************************************************************************************************************/
unsigned char* kangarooSpeedReadback(unsigned char* buffer, int fd)
{
    int charWritten = UARTWrite(fd, buffer, strlen(buffer));
    int charRead = UARTRead(fd, buffer, 50);
    kangarooReadbackError(buffer);
    return buffer;
}

/*********************************************************************************************************************************************
 * Function: kangarooReadbackError()
 * 
 * Description:
 *  Input:  The output of the Kangaroo for a readback request
 *  Output: Either some error message or passes back the input.
 * Notes:
 * 
*********************************************************************************************************************************************/
unsigned char* kangarooReadbackError(unsigned char* readback)
{
    unsigned char* errorMsg;
    
    if( strcmp(readback, "E1") == 0 )
    {
        errorMsg = "Channel Not Started";
        return errorMsg;
    }
    else if( strcmp(readback, "E2") == 0 )
    {
        errorMsg = "Channel Not Homed";
        return errorMsg;
    }
    else if( strcmp(readback, "e2") == 0 )
    {
        errorMsg = "Homing In Progress";
        return errorMsg;
    }
    else if( strcmp(readback, "E3") == 0 )
    {
        errorMsg = "Control Error, Channel Disabled";
        return errorMsg;
    }
    else if( strcmp(readback, "E4") == 0 )
    {
        errorMsg = "Wrong Mode, Tune Again to Use Mode";
        return errorMsg;
    }
    else if( strcmp(readback, "E5") == 0 )
    {
        errorMsg = "Readback Command Not Recognized";
        return errorMsg;
    }
    else if( strcmp(readback, "E6") == 0 )
    {
        errorMsg = "Kangaroo Lost Communication";
        return errorMsg;
    }

    return readback;
    
}