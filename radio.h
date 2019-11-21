#ifndef RADIO_H
#define RADIO_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>

//#include "errors.h"
#define FRAME_PAYLOAD_SIZE 72
#define RECIEVER_ADRS "127.0.0.1"

extern struct sockaddr_in ClientService;
extern int mySocket;

int radio_init ( int addr );
int radio_send ( int dst , char * data , int len );
int radio_recv ( int * src , char * data , int to_ms );

# endif // _RADIO_H_
