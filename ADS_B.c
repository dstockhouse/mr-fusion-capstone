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
#include <string.h>

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
    uint8_t startFlg;
    uint8_t length;
    uint8_t sequence;
    uint8_t sysID;
    uint8_t component;
    uint8_t messageID;
} MsgHeader;

typedef struct {
    uint32_t ICAO_adress;
    int32_t lat; // Latitude in degrees*1E7
    int32_t lon; // Longitude in degrees * 1E7
    int32_t altitude; // Altitude in Meters *1E3
    uint16_t heading; // Course over ground in degrees *10E2
    uint16_t hor_velocity; // Horizontal Velocity (m/s) * 1E2
    uint32_t ver_velocity; // Vertical Velocity (m/s) * 1E2
    uint16_t validFlags; // Flags
    uint16_t squawk; // Squawk code (0xFFFF = no code)
    uint8_t altitude_type; // Reference point for altitude
    char callsign[9]; // Callsign
    uint8_t emitter_type; //Emitter category
    uint8_t tslc; // Time since last communication
} MsgData246;

typedef struct {
    uint8_t checksumA;
    uint8_t checksumB;
} MsgFooter;

typedef struct {
    MsgHeader header;
    MsgData246 data246;
    MsgFooter footer;
} Message246;

/*
* Function -> main()
* retval -> 1 on success
*/
int main(void) {
    uint32_t rv = 0;
    uint8_t* testHeader = "\xfe\x26\x28\x00\x00\xf6"; // just the header for now
    uint8_t* testData = "\xFF\xEA\x00\x00\xC0\xE2\xA1\x14\x4B\x6B\xF9\xBC\x10\xCA\x00\x00\x00\x00\x00\x00\x00\x1F\x80\x00\x00\x00\x49\x43\x41\x52\x55\x53\x31\x00\x00\x0E\x01\x65\xDF";
    uint8_t* testFooter = "\x4a\x39";
    MsgHeader header;
    MsgData246 data;

    // Enough room for all data in the file
#define FILE_SIZE 221184
    char fileData[FILE_SIZE];
    int rc, i = 0;

    // Binary file descriptor
    FILE *file;

    // 'b' not strictly necessary, but won't hurt
    file = fopen("SampleData/ADSB_log.bin", "rb");

    // Read binary data into local variable
    rc = fread(fileData, 1, FILE_SIZE, file);
    printf("Read %d bytes\n\n", rc);

    while(i < rc) {

	    // Seek to next start of packet
	    for( ; i < rc - 38 && fileData[i] != 0xfe && fileData[i + 1] != 0x26 && fileData[i + 5] != 246; i++) ;

	    if(i >= rc - 38) {
		    return 0;
	    }

	    printf("Parsing index %d (0x%x)\n", i, i);

	    // rv = parseHeader(testHeader, &header);
	    rv = parseHeader(&(fileData[i]), &header);
	    printf("Header\n");
	    printf("start flg: %d\n", header.startFlg);
	    printf("message ID: %d\n", header.messageID);
	    printf("lenght: %d\n", header.length);

	    // rv = parseData(testData, &data);
	    rv = parseData(&(fileData[i + 6]), &data);
	    printf("data\n");
	    printf("Callsign: %s\n\n", data.callsign);

	    i++;

    }

}

/*
* Function -> parseData
* Purpose -> parse adsb data for a single message
* Input -> single data binary
* Output -> Pointer to struct of data field values
*/
int parseData(uint8_t* data, MsgData246* msgData) {
    int idx = 0;

    printf("Data is:\n");
    for(idx = 0; idx < 38; idx++) {
	    printf("%3d (%02d): (0x%x)\n", idx + 6, idx, data[idx]);
    }
    printf("\n");

    msgData->ICAO_adress = ((data[0] << 0x18) | (data[1] << 0x10) |
                            (data[2] << 0x08) | (data[3]));
    msgData->lat = ((data[4] << 0x18) | (data[5] << 0x10) |
                    (data[6] << 0x08) | (data[7]));
    msgData->lon = ((data[8] << 0x18) | (data[9] << 0x10) |
                    (data[10] << 0x08) | (data[11]));
    msgData->altitude = ((data[12] << 0x18) | (data[13] << 0x10) |
                         (data[14] << 0x08) | (data[15]));
    msgData->heading = ((data[16] << 0x08) | (data[17]));
    msgData->hor_velocity = ((data[18] << 0x08) | (data[19]));
    msgData->ver_velocity = ((data[20] << 0x08) | (data[21]));
    msgData->validFlags = ((data[22] << 0x08) | (data[23]));
    msgData->squawk= ((data[24] << 0x08) | (data[25]));
    msgData->altitude_type = data[26];
    // printf("%c\n%c\n", data[26], data[27]);
    for (idx = 0;idx < 9;idx++) {
        printf("idx: %d, d: (0x%x)\n", idx, data[27+idx]);
        msgData->callsign[idx] = data[27+idx];
    }
    msgData->emitter_type = data[36];
    msgData->tslc = data[37];
}


int parseHeader(uint8_t* message, MsgHeader* header) {
	int idx;
    printf("Data is:\n");
    for(idx = 0; idx < 6; idx++) {
	    printf("%3d: (0x%x)\n", idx, message[idx]);
    }
    printf("\n");

    // Find Message id
    header->startFlg = message[0]; // Find start flag

    // verify that start flag is expected value
    if (header->startFlg == 0xfe) {
        // parse the header
        header->length = message[1];
        header->sequence = message[2];
        header->sysID = message[3];
        header->component = message[4];
        header->messageID = message[5];
        return 1;
    }
    return -1;
}
