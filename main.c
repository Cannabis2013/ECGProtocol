#include <stdio.h>
#include "radio.h"
#include <string.h>

#include "ecgprotocol.h"

#define PORT_SRC_ARG "--PORT-SRC"
#define PORT_DST_ARG "--PORT-DST"
#define MESSAGE "--MESSAGE"
#define SERVER_ROLE "--ROLE-SERVER"
#define CLIENT_MODE "--ROLE-CLIENT"
#define TIME_OUT_FLAG "--SET-TIMEOUT"

int main(int argc, char *argv[])
{
    char *data = malloc(sizeof (char*));

    ushort src_port = 22500;
    ushort dst_port = 45000;
    int timeout = 2500;

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
       else if(strncmp(TIME_OUT_FLAG,argv[i],sizeof (TIME_OUT_FLAG)) == 0)
       {
           timeout = (ushort) strtol(argv[i+1],NULL,10);
           i = (i < argc) ? ++i : i;
       }
       else if(strncmp(MESSAGE,argv[i],sizeof (MESSAGE)) == 0)
       {
           strcpy(data,argv[i+1]);
           i = (i < argc) ? ++i : i;
       }
    }

    printf("\nSource port: %d\n",src_port);
    printf("Destination port: %d\n",dst_port);
    printf("DATA: %s\n",data);
    char *recv_data = NULL;
    ecg_init((int) src_port);
    ecg_recieve(0,recv_data,0,timeout);

    return 0;
}
