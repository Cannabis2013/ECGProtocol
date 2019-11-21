#include "radio.h"

int radio_init(int addr)
{
    struct in_addr adrs;
    adrs.s_addr = (uint) addr;
    if(inet_aton(LOCALHOST,&adrs) == 0)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return -1;
    }

    /* Verify that the port is within the desired range
     * In this case we don't want to use ports below 100 or above 60000.
     */

    if(addr < 100 || addr > 60000)
    {
        printf("%s","Not a valid port. Did you enter the addres of your butt you dum liberal?");
        return -1;
    }

    ClientService.sin_family = AF_INET;
    ClientService.sin_port = htons(LOCALADRESS);
    ClientService.sin_addr.s_addr =(uint) adrs.s_addr;

    mySocket = socket(AF_INET,SOCK_STREAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return -1;
    }

    return 0;
}

int radio_send(int dst, char *data, int len)
{
    if(inet_aton(&dst,NULL) == 0)
    {
        // TODO: Implement some error handling or specifikation
        return -1;
    }

    int connection = connect(mySocket,(struct sockaddr *)&ClientService,sizeof (ClientService));

    // Check if connection is established and handle the error if necessary
    if(connection < 0)
    {
        // TODO: Implement error handling
        // NOTE: The connection variable is -1 if socket failed to establish connection to the destination
        return -1;
    }
    if(send(mySocket,NULL,0,0) < 0)
    {

    }
}
