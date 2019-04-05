/****************************************************************************\
 *
 * File:
 * 	adsb_parser.h
 *
 * Description:
 * 	Function and type declarations and constants for adsb_parser.c
 *
 * Author:
 * 	Joseph Kroeker & David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/19
 *
\***************************************************************************/

#ifndef __ADSB_PARSER_H
#define __ADSB_PARSER_H

#include "../buffer/buffer.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
	int32_t altitude_inv; // Altitude in Meters *1E3
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

int parseData(uint8_t* data, MsgData246* msgData, int verbose);

int printData(MsgData246 *data);

int parseHeader(uint8_t* message, MsgHeader* header, int verbose);

int printHeader(MsgHeader *header);

int parseBuffer(BYTE_BUFFER *buf, MsgHeader *header, MsgData246 *data);

int logDataRaw(LOG_FILE *logFile, uint8_t *data);

int logDataParsed(LOG_FILE *logFile, MsgData246 *data);

#endif

