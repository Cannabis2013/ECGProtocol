#ifndef ECGPROTOCOL_H
#define ECGPROTOCOL_H

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
    char _data[0];
}data ;


typedef union
{
    char    _raw[FRAME_PAYLOAD_SIZE];

    header  _header;
    data    _data;
}frame;

void verifyChecksum(void)
{
    // TODO: Implements an algorithm that verifies the integrity of recieved data.
}

void ecg_send(int dst, char *packet,int len);

#endif // ECGPROTOCOL_H
