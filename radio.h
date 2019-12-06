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
#include <string.h>

#include "custom_timer.h"

#define SLEEP_TIME 100

// Used to silence warnings related to unused variables
#define VAR_UNUSED(X) X = X

#define FRAME_SIZE 240 // Total size of the frame to be transmitted
#define FRAME_PAYLOAD_SIZE 207 // Raw data size
#define CHUNK_SIZE 218 // Raw data size + additional meta overhead

// Device adress related
#define LOCALHOST "127.0.0.1"
#define LOCALADRESS 55000

#define AWAIT_CONTIGUOUS 0
#define AWAIT_TIMEOUT 1

#define SEED 5
uint unique_adress;

typedef struct
{
    char err_msg[128];
    int code;
}radio_err;

radio_err radio_error;

struct sockaddr_in LocalService;

// The various return types when recieveing or sending packets/frames
#define TIMEOUT -1
#define CONNECTION_ERROR -2
#define SOCKET_ERROR -3
#define INVALID_ADRESS -4
#define CONNECTION_REQUEST_IGNORED -5
#define CONNECTION_HANDSHAKE_FAILED -6

// Used for ensuring one peer-to-peer relation at a time
typedef struct
{
    uint peer_id;
    ushort peer_adrs;
    int connection_established;
}REMOTE_META;

REMOTE_META remote;


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
    ushort          checksum; // 2 bytes allocated
    char            payload[CHUNK_SIZE]; // 218 bytes allocated

}Frame; // 10 + 4 + 6 + 139  = 22 + 139= 161 bytes total allocated for this structure

typedef union
{
    char    raw[FRAME_SIZE];
    Frame   frame; // 162 bytes allocated

}Frame_PDU;


static int mySocket;

unsigned int permuteQPR(unsigned int x);

int radio_init ( int addr );
int radio_send ( int dst , char * data , int len );
int radio_recv ( int * src , char * data , int to_ms);

void cp_data(char*dst, char*src, uint src_len);

char *integertoc(uint number);
# endif // _RADIO_H_
