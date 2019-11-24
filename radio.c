#include "radio.h"

int radio_init(int addr)
{
    if(inet_aton(LOCALHOST,&LocalService.sin_addr) == 0)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return -3;
    }

    /* Verify that the port is within the desired range
     * In this case we don't want to use ports below 100 or above 60000.
     */

    if(addr < 1024 || addr > 60000)
    {
        printf("%s","Not a valid port. Did you enter the addres of your butt you dum liberal?");
        return -3;
    }

    LocalService.sin_family = AF_INET;
    LocalService.sin_port = htons(LOCALADRESS);

    mySocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return -3;
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

    ssize_t byte_transmitted = sendto(mySocket,data,(uint) len,0,
                                      (struct sockaddr *)&remoteService,sizeof (remoteService));

    if(byte_transmitted < 0)
    {
        // TODO: Implement error handling
        return -2;
    }

    return (int) byte_transmitted;
}

int radio_recv(int *src, char *data, int to_ms)
{
    /*
     * TODO: Need to implement some error functionality
     */

    // Check if adress is valid and initialize struct for later use
    struct sockaddr_in remoteService;
    if(inet_aton(LOCALHOST,&remoteService.sin_addr) == 0)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return -4; // INVALID_ADRESS
    }

    // Check if destination port is within our desired interval limits
    if(*src < 100 || *src > 60000)
    {
        printf("%s","Not a valid port. Did you enter the addres of your butt you dum liberal?");
        return -4; // INVALID_ADRESS
    }

    remoteService.sin_family = AF_INET;
    remoteService.sin_port = (in_port_t) *src;

    int connection = connect(mySocket,(struct sockaddr *)&remoteService,sizeof (remoteService));
    if(connection < 0)
        return -2; // CONNECTION_ERROR


    if(to_ms == 0)
    {
        ssize_t bytes_recieved = recv(mySocket,data,FRAME_PAYLOAD_SIZE,0);
        if(bytes_recieved == 0)
            return 0;
        else
            return (int) bytes_recieved;
    }

    start_timer();
    while (time_elapsed() <= to_ms) {
        ssize_t bytes_recieved = recv(mySocket,data,FRAME_PAYLOAD_SIZE,0);
        if(bytes_recieved > 0)
        {
            stop_timer();
            return (int) bytes_recieved;
        }
    }
    stop_timer();
    return -1; // TIMEOUT
}
