#include "radio.h"

int radio_init(int addr)
{
    struct in_addr adrs;
    adrs.s_addr = addr;
    char *data = inet_ntoa(adrs);
    if(inet_aton(data,&adrs) == 0)
    {
        // TODO: Implement some validy confirmation
    }

    /* Verify that the port is within the desired range
     * In this case we don't want to use ports below 100 or above 60000.
     */

    if(addr < 100 && addr > 60000)
    {
        // TODO: Implement some validy confirmation
    }

    ClientService.sin_family = AF_INET;
    ClientService.sin_port = htons(3377);
    ClientService.sin_addr.s_addr =(uint) adrs.s_addr;

    mySocket = socket(AF_INET,SOCK_STREAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return -1;
    }

    //int connection = connect(mySocket,(struct sockaddr *)&ClientService,sizeof (ClientService));
    return 0;
}
