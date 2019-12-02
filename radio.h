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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "custom_timer.h"

#define FRAME_PAYLOAD_SIZE 128
#define FRAME_OVERHEAD_SIZE 22
#define TRANSFER_RATE_BITS 19200
#define TRANSFER_RATE_BYTES (TRANSFER_RATE_BITS/8)

// We may find a way to generate a unique host adress for each devices
#define LOCALHOST "127.0.0.1"
#define LOCALADRESS 55000


#define SEED 0
uint unique_adress;

struct sockaddr_in LocalService;
#define TIMEOUT -1
#define CONNECTION_ERROR -2
#define SOCKET_ERROR -3
#define INVALID_ADRESS -4

typedef struct
{
    ushort src; // 2 bytes allocated
    ushort dst; // 2 bytes allocated
    u_int8_t lenght; // 1 bytes allocated
    u_int8_t protocol; // 1 bytes allocated
}Frame_Header; // 6 bytes total allocated for this structure

typedef struct
{
    char            preAmble[10]; // 10 bytes allocated
    uint            unique_adress; // 4 bytes allocated
    Frame_Header    header; // 6 bytes allocated
    char            payload[128]; // 128 bytes allocated
    ushort          checksum; // 2 bytes allocated

}Frame; // 10 + 4 + 6 + 128 + 2 = 150 bytes total allocated for this structure

typedef union
{
    char    raw[150];
    Frame   frame; // 150 bytes allocated

}Frame_PTU;


static int mySocket;

unsigned int permuteQPR(unsigned int x);

int radio_init ( int addr );
int radio_send ( int dst , char * data , int len );
int radio_recv ( int * src , char * data , int to_ms);

void cp_data(char*dst, char*src, uint src_len);

char *integertoc(uint number);
# endif // _RADIO_H_
