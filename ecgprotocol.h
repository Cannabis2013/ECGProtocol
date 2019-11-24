#ifndef ECGPROTOCOL_H
#define ECGPROTOCOL_H

#include "radio.h"
#include <time.h>

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

#define type_t char

/*
 * Frame type abbreviations:
 *      - NIT -- Not IniTialized
 *      - EOT -- End Of Transmission
 */

#define NIN char 0;
#define EOT char 1
#define DATA char 2
#define META char 3
#define INIT char 4

typedef struct
{
    type_t  _type;
}type;

typedef struct
{
    type    _type; // Allocates 1 bytes for type identification
    short     _src; // Allocates 2 bytes for source adress
    short     _dst; // Allocates 2 bytes for destination adress
    char    _protocol; // Allocates 1 byte for protocol identification

}header;

typedef struct
{
    type    _type;
    char    _data[0];
}data ;


typedef union
{
    char    _raw[FRAME_PAYLOAD_SIZE];

    header  _header;
    data    _data;
}Frame;

void verifyChecksum(void);

void ecg_send(int dst, char *data,int len);
void ecg_recieve(int src,char *data,int _timeout);

#endif // ECGPROTOCOL_H
