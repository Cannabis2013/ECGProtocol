#ifndef ECGPROTOCOL_H
#define ECGPROTOCOL_H

#include <time.h>
#include <string.h>
#include "radio.h"


/*
 * If anyone has seen the example code available on inside.dtu,
 * you probably took note of the usage of the union structure.
 * In my opinion it might prove useful in terms of flexibility, memory efficiency and  most important
 * it makes it easier to identify packet type and signal state transitions. We should definitely go with the
 * union type in combination with ordinary structures IMO.
 *
 * TODO's:
 *      - We have to implements some checksum algorithm we first have to figure out.
 */

#define KEY1 0x9A
#define KEY2 0xB8


static int channel_established = 0;

/*
 * Error handling:
 *  - Error structure containing error code and description
 */

typedef struct
{
    int error_code;
    char error_description[0];
}Error;

static Error error;

// Remote meta information structure

#define CONNECTION_INIT_ATTEMPT 4
#define CONNECTION_SEND_ATTEMPT 3
#define CONNECTION_AWAIT_ATTEMPT 3
#define CONNECTION_FINAL_ATTEMP 8
#define CONNECTION_LISTEN_ATTEMPT 1

#define type_t char

/*
 * PTU types
 */

#define CHUNK '0'
#define META '1'
#define INIT '2'
#define ACK '3'
#define P_ACK '4'
#define LAST_CHUNK '5'

typedef struct
{
    type_t  type;
}Type;

typedef struct
{
    Type    type; // Allocates 1 bytes for type identification
    ushort  src; // Allocates 2 bytes for source adress
    ushort  dst; // Allocates 2 bytes for destination adress
    uint    total_size; // Allocates 4 bytes for total chunk size
    char    protocol; // Allocates 1 byte for protocol identification
    unsigned int magic_key; // Allocates 4 byres for unique identification

}Header;

typedef struct
{
    Type    type; // Allocates 1 byte
    char    data[FRAME_PAYLOAD_SIZE]; // 128 bytes allocated
    uint    chunk_size; // Allocates 4 bytes
    ushort    checksum; // Allocates 2 bytes
}Chunk; // Allocates 135 bytes


typedef union
{
    char    raw[CHUNK_SIZE];

    Header  header;
    Chunk    chunk;
}Packet;

ushort generateChecksum(char *msg, ushort key);

int try_send(Packet *packet, int adrs_reciever, int connection_attempts);
int await_reply(Packet *buffer, int timeout, int connection_attempts, int mode);

int ecg_init ( int addr );
int ecg_send(int dst, char *data, int len, int to_ms);
int ecg_recieve(int src, char *data, int len, int to_ms);

#endif // ECGPROTOCOL_H
