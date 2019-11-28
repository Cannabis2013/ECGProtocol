/*
 *      Notes for the group regarding the use of the sys/socket library.
 *
 *      For information about functions and their return values please visit the following link:
 *
 *          Url: https://pubs.opengroup.org/onlinepubs/7908799/xns/connect.html
 *
 *      The link contains some usefull description of functions and definitions that might prove usefull
 *      with respect to return values and their meaning..
 */


#ifndef RADIO_H
#define RADIO_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <uuid/uuid.h>

#include "custom_timer.h"

#define FRAME_PAYLOAD_SIZE 72

// We may find a way to generate a unique host adress for each devices
#define LOCALHOST "127.0.0.1"
#define LOCALADRESS 55000


/* This part isn't so easy. We assume that the radio chip has its own unique adressing
 * in forms of a number that is ordered by manufaction
 */

#define CHIP_NUMBER = 0
#define UNIQUE_ADRESS = permuteQPR(permuQPR(CHIP_NUMBER) )^0x5bf03635;

#define TIMEOUT -1
#define CONNECTION_ERROR -2
#define SOCKET_ERROR -3
#define INVALID_ADRESS -4


static struct sockaddr_in LocalService;
static int mySocket;

unsigned int permuteQPR(unsigned int x);

int radio_init ( int addr );
int radio_send ( int dst , char * data , int len );
int radio_recv ( int * src , char * data , int to_ms);


# endif // _RADIO_H_
