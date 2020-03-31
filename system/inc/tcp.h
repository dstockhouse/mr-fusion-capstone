/****************************************************************************
 *
 * File:
 *      tcp.h
 *
 * Description:
 *      Function and type declarations and constants for tcp.c
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 03/04/2020
 *
 * Revision 0.2
 *      Last edited 03/19/2020
 *      Changed some function interfaces
 *
 ***************************************************************************/

#ifndef __TCP_H
#define __TCP_H

int TCPClientInit(void);

int TCPClientTryConnect(int sock_fd, char *ipAddr, int port);

int TCPServerInit(char *ipAddr, int port);

int TCPServerTryAccept(int sock_fd);

int TCPSetNonBlocking(int sock_fd);

int TCPRead(int sock_fd, unsigned char *buf, int length);

int TCPWrite(int sock_fd, unsigned char *buf, int length);

int TCPClose(int sock_fd);

#endif // __TCP_H

