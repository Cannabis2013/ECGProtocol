#ifndef ECGPROTOCOL_H
#define ECGPROTOCOL_H

#include <time.h>
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

/*
 * Error handling:
 *  - Error structure containing error code and description
 */

typedef struct
{
    int error_code;
    char error_description[128];
}Error;

static Error error;

// Remote meta information structure

#define CONNECTION_INIT_ATTEMPT 2
#define CONNECTION_SEND_ATTEMPT 2
#define CONNECTION_FINAL_ATTEMP 8

#define type_t char

/*
 * PDU types
 */

#define CHUNK '0'
#define INIT '1'
#define ACK '2'
#define P_ACK '3'
#define COMPLETE '4'
#define P_CHECKSUM_FAIL '5'
#define RESEND '6'

typedef struct
{
    type_t  type;
}Type;

typedef struct
{
    Type    type; // Allocates 1 bytes for type identification

}Header; // Allocates 1 bytes

typedef struct
{
    uint    size; // Allocates 4 bytes
    uint    t_send; // Allocates 4 bytes
    int     seal; // Allocates 4 bytes
    Type    type; // Allocates 1 byte
    char    data[FRAME_PAYLOAD_SIZE]; // 205 bytes allocated
}Chunk; // Allocates 218 bytes


typedef union
{
    char    raw[CHUNK_SIZE]; // Allocates 218 bytes

    Header  header;
    Chunk   chunk;
}Packet;

typedef struct
{
    int bytes_sent;
    int bytes_recv;

    Packet p_recv;
}TRANSMIT_DETAILS;

#define CONNECTION_TYPE int
#define INDBOUND 0
#define OUTBOUND 1

typedef struct
{
    int peer_adrs;
    CONNECTION_TYPE type;
    int bytes_transmitted;
    clock_t session_start_clock;
    clock_t session_end_clock;
    clock_t transmission_start_clock;
    clock_t transmission_end_clock;

    int packets_lost;
}STATISTICS;

STATISTICS session_statistics;
int fingerprint(char *msg, int key, int len);
void printArray(char *arr, int len);
int send_and_await_reply(Packet*packet, int adrs_reciever, int connection_attempts, int timeout, int mode, TRANSMIT_DETAILS *t);
int try_send(Packet *packet, int adrs_reciever, int connection_attempts);
int await_reply(Packet *buffer, int timeout, int mode);

int ecg_init ( int addr );
int ecg_send(int dst, char *packet, int len, int to_ms);
int ecg_recieve(int src, char *packet, int len, int to_ms);

#endif // ECGPROTOCOL_H
