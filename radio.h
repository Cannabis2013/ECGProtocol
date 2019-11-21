/*
 *      Notes for the group regarding the use of the sys/socket library.
 *
 *      For information about functions and their return values please visit the following:
 *
 *          Url: https://pubs.opengroup.org/onlinepubs/7908799/xns/connect.html
 *
 *      The link contains some usefull description of functions and definitions that might prove usefull
 *      in relation to usage and further clarification.
 *
 *      My issues (Martin Hansen) can be outlined as follows:
 *          - Do we need to implement the loop here?
 *              - If yes; should we make use of the 'pthread' library (threading)?
 *          - Should we just stick to a local domain driven interface?
 *              (We use ports as localadress as specified in the document and ignore internet functionality)
 *          - Shouldn't we force Peter to make the whole assignment blindfolded? :D
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

//#include "errors.h"
#define FRAME_PAYLOAD_SIZE 72
#define LOCALHOST "127.0.0.1"
#define LOCALADRESS 55000

extern struct sockaddr_in ClientService;
extern int mySocket;

extern const int local_adress;

int radio_init ( int addr );
int radio_send ( int dst , char * data , int len );
int radio_recv ( int * src , char * data , int to_ms );

# endif // _RADIO_H_
