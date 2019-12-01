#include <stdio.h>
#include "radio.h"
#include <string.h>

#include "ecgprotocol.h"

#define PORT_SRC_ARG "--PORT-SRC"
#define PORT_DST_ARG "--PORT-DST"
#define MESSAGE "--MESSAGE"

int main(int argc, char *argv[])
{
    char *data = malloc(sizeof (char*));

    uint src_port = 42000;
    uint dst_port = 45000;

    for (int i = 1; i < argc; i++) {
       if(strncmp(PORT_SRC_ARG,argv[i],sizeof (PORT_SRC_ARG)) == 0)
       {
           printf("SRC branch:  %d\n",i);
           src_port = (uint) strtol(argv[i + 1],NULL,10);
           i = (i < argc) ? ++i : i;
       }
       else if(strncmp(PORT_DST_ARG,argv[i],sizeof (PORT_DST_ARG)) == 0)
       {
           printf("DST branch: %d\n",i);
           dst_port = (uint) strtol(argv[i+1],NULL,10);
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
    /*
    ecg_init(src_port);
    ecg_send(dst_port,data,sizeof (data),5000);

    */
    return 0;
}
