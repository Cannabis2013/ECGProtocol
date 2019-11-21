#include "radio.h"

int radio_init(int addr)
{
    struct in_addr adrs;
    adrs.s_addr = addr;
    char *data = inet_ntoa(adrs);
    if(inet_aton(data,&adrs) == 0)
    {
        // TODO: Implement some validy check
    }

    ClientService.sin_family = AF_INET;
    ClientService.sin_port = htons(3377);
    ClientService.sin_addr.s_addr =(uint) addr;

    mySocket = socket(AF_INET,SOCK_STREAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return -1;
    }

    //int connection = connect(mySocket,(struct sockaddr *)&ClientService,sizeof (ClientService));
    return 0;
}
