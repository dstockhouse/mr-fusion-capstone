/****************************************************************************
 *
 * File:
 * 	tcp.h
 *
 * Description:
 * 	Function and type declarations and constants for tcp.c
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 03/04/2020
 *
 ***************************************************************************/

#ifndef __TCP_H
#define __TCP_H

int TCPInitClient(char *ipAddr, int port);

int TCPInitServer(char *ipAddr, int port);

int TCPRead(int sock_fd, unsigned char *buf, int length);

int TCPWrite(int sock_fd, unsigned char *buf, int length);

int TCPClose(int sock_fd);

#endif // __TCP_H

