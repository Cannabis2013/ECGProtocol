#include <stdio.h>
#include "radio.h"
#include <string.h>

#include "ecgprotocol.h"

#define PORT_SRC_ARG "--PORT-SRC"
#define PORT_DST_ARG "--PORT-DST"
#define MESSAGE_ARG "--MESSAGE"
#define SERVER_ROLE_ARG "--ROLE-SERVER"
#define CLIENT_MODE_ARG "--ROLE-CLIENT"
#define TIME_OUT_ARG "--SET-TIMEOUT"

#define SERVER_ROLE 0
#define CLIENT_ROLE 1

int main(int argc, char *argv[])
{
    char outbound_data[4096];

    ulong msg_size = 0;

    ushort src_port = 35000;
    ushort dst_port = 22500;
    int timeout = 2500;
    int role = CLIENT_ROLE;

    for (int i = 1; i < argc; i++) {
       if(strncmp(PORT_SRC_ARG,argv[i],sizeof (PORT_SRC_ARG)) == 0)
       {
           src_port = (ushort) strtol(argv[i + 1],NULL,10);
           i = (i < argc) ? ++i : i;
       }
       else if(strncmp(PORT_DST_ARG,argv[i],sizeof (PORT_DST_ARG)) == 0)
       {
           dst_port = (ushort) strtol(argv[i+1],NULL,10);
           i = (i < argc) ? ++i : i;
       }
       else if(strncmp(TIME_OUT_ARG,argv[i],sizeof (TIME_OUT_ARG)) == 0)
       {
           timeout = (ushort) strtol(argv[i+1],NULL,10);
           i = (i < argc) ? ++i : i;
       }
       else if(strncmp(SERVER_ROLE_ARG,argv[i],sizeof (SERVER_ROLE_ARG)) == 0)
       {
           role = SERVER_ROLE;
       }
       else if(strncmp(MESSAGE_ARG,argv[i],sizeof (MESSAGE_ARG)) == 0)
       {
           strcpy(outbound_data,argv[i+1]);
           msg_size = (ulong) strtol(argv[i+2],NULL,10);
           i = (i < argc) ? ++i : i;
       }
    }

    char inbound_data[msg_size];

    printf("Initial state:\n");
    printf("\tSource port: %d\n",src_port);
    printf("\tDestination port: %d\n",dst_port);
    printf("\tData out: %s\n",outbound_data);
    printf("\tTransmission size: %lld\n",msg_size);

    // Initialize the radio chip
    ecg_init((int) src_port);

    if(role == SERVER_ROLE)
    {
        printf("Listening session initated. Now listens on port: %d\n",src_port);
        ecg_recieve(0,inbound_data,0,timeout);
        printf("Recieved data: %s\n",inbound_data);
        return 0;
    }

    ecg_send(dst_port,outbound_data,msg_size,timeout);

    return 0;
}
