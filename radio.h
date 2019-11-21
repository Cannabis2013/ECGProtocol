#ifndef RADIO_H
#define RADIO_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef _RADIO_H_
#define _RADIO_H_

//#include "errors.h"
#define FRAME_PAYLOAD_SIZE 72
#define RECIEVER_ADRS "127.0.0.1"

int radio_init ( int addr );
int radio_send ( int dst , char * data , int len );
int radio_recv ( int * src , char * data , int to_ms );

# endif // _RADIO_H_

#endif // RADIO_H
