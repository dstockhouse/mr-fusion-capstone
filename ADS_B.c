
/****************************************************************************\
 *
 * File:
 * 	ADS_B.c
 *
 * Description:
 *  Interfaces with the ADS-B reciever
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 1.1
 * 	Last edited 6/7/18
 *
\***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define COM_PORT "/dev/TTYUSB0"

// Altitude type options
enum alt_type { Pressure, Geometric };
// Emitter type options
enum emitter_type { No_Type_Info, Light, Small, Large, High_Vortex_Large,
                    Heavy, Highly_Manuv, Rotocraft, Unassigned, Glider,
                    Ligher_Air, Parachute, Ultra_Light, Unassigned2, UAV,
                    Space, Unassigned3, Emergency_Surface, Service_Surface,
                    Point_Obstacle };
// Validity flag options (1 = valid)
enum flags { LatLon = 0x0001, Altitude = 0x0002, Heading = 0x0004,
             Velocity = 0x0008, Callsign =0x0010, Ident = 0x0020,
             Simulated_Report = 0x0040, Vertical_Velocity = 0x0080,
             Baro = 0x0100, Source_UAT = 0x8000 };

typedef struct {
    int messageID;
    int payloadLength;
    int CRCExtra;
} MsgHeader;

typedef struct {
    uint32_t ICAO_adress;
    int32_t lat; // Latitude in degrees*1E7
    int32_t lon; // Longitude in degrees * 1E7
    int32_t altitude; // Altitude in Meters *1E3
    uint16_t heading; // Course over ground in degrees *10E2
    uint16_t hor_velocity; // Horizontal Velocity (m/s) * 1E2
    int ver_velocity; // Vertical Velocity (m/s) * 1E2
    uint16_t validFlags; // Flags
    uint16_t squawk; // Squawk code (0xFFFF = no code)
    uint8_t altitude_type; // Reference point for altitude
    char callsign[9]; // Callsign
    uint8_t emitter_type; //Emitter category
    uint8_t tslc; // Time since last communication
} MsgData246;

typedef struct {
    MsgHeader header;
    MsgData data;

} Message246;
/*
* Function -> main()
* retval -> 1 on success
*/
int main(void) {
    int conAttempts = 0; // connection Attemps (limit = 3)
    int rv = 0; // Ret val from other functions
    for (conAttempts = 0; (conAttempts < 3) && (rv != 1); conAttempts++) {
        rv = connectPort(); // Attempt Connection to port
    }

}

/*
* Function -> connectPort
* retval -> 1 on success, 0 on failure
*/
int connectPort(void){

}

/*
* Function -> adsbParser
* Purpose -> read through data and parse adsb messages
* Input -> single message binary
* Output -> Pointer to struct of message field values
*/
int adsbParser(char* message) {

    return 0;
}
