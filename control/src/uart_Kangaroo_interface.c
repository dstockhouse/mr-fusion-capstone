#include "uart.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

void kangarooInterfaceSetup()
{
    unsigned char temp1[] = "1,start";
    unsigned char temp2[] = "2,start";
    unsigned char temp3[] = "1,units 100 cm = 2048 lines";
    unsigned char temp4[] = "2,units 100 cm = 2048 lines";

    int fd = UARTInit("/dev/ttyS0", 9600);
    int charWritten = UARTWrite(fd, temp1, strlen(temp1));
    int charWritten = UARTWrite(fd, temp2, strlen(temp2));
    int charWritten = UARTWrite(fd, temp3, strlen(temp3));
    int charWritten = UARTWrite(fd, temp4, strlen(temp4));
}

void kangarooSpeedControl(unsigned char* buffer, int fd)
{
    int charWritten = UARTWrite(fd, buffer, strlen(buffer));
}

unsigned char* kangarooSpeedReadback(unsigned char* buffer, int fd)
{
    int charWritten = UARTWrite(fd, buffer, strlen(buffer));
    int charRead = UARTRead(fd, buffer, 50);
    kangarooReadbackError(buffer);
    return buffer;
}

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