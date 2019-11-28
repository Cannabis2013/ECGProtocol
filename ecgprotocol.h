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

static int inital_send = 1;
static int initial_recieve = 1;


/*
 * Error handling:
 *  - Erorr structure containing error code and description
 */

typedef struct
{
    int error_code;
    char error_description[0];
}Error;

#define type_t char

/*
 * Frame type abbreviations:
 *      - NIT -- Not IniTialized
 *      - EOT -- End Of Transmission
 */

#define NIN '0';
#define EOT '1'
#define DATA '2'
#define META '3'
#define INIT '4'
#define ACKWM '5'

typedef struct
{
    type_t  _type;
}Type;

typedef struct
{
    Type    _type; // Allocates 1 bytes for type identification
    short   _src; // Allocates 2 bytes for source adress
    short   _dst; // Allocates 2 bytes for destination adress
    char    _protocol; // Allocates 1 byte for protocol identification
    unsigned int _magic_key; // Allocates 4 byres for unique identification

}Header;

typedef struct
{
    Type    _type;
    char    _data[0];
}Data ;


typedef union
{
    char    _raw[FRAME_PAYLOAD_SIZE];

    Header  _header;
    Data    _data;
}Frame;

void verifyChecksum(void);

int ecg_send(int dst, char *data,int len);
int ecg_recieve(int src,char *data,int _timeout);

#endif // ECGPROTOCOL_H
