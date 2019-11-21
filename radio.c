#include "radio.h"

int radio_init(int addr)
{
    if(inet_aton(LOCALHOST,&LocalService.sin_addr) == 0)
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

    LocalService.sin_family = AF_INET;
    LocalService.sin_port = htons(LOCALADRESS);

    mySocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return -1;
    }

    return 0;
}

int radio_send(int dst, char *data, int len)
{
    // Check if adress is valid and initialize struct for later use
    struct sockaddr_in remoteService;
    if(inet_aton(LOCALHOST,&remoteService.sin_addr) == 0)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return -1;
    }

    // Check if destination port is within our desired interval limits
    if(dst < 100 || dst > 60000)
    {
        printf("%s","Not a valid port. Did you enter the addres of your butt you dum liberal?");
        return -1;
    }

    remoteService.sin_family = AF_INET;
    remoteService.sin_port = (in_port_t) dst;

    ssize_t byte_transmitted = sendto(mySocket,data,(uint) len,0,(struct sockaddr *)&remoteService,sizeof (remoteService));

    if(byte_transmitted < 0)
    {
        // TODO: Implement error handling
        return -1;
    }



    return (int) byte_transmitted;
}
