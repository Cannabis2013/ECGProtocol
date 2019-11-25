#include "radio.h"

int radio_init(int addr)
{
    if(inet_aton(LOCALHOST,&LocalService.sin_addr) == 0 || addr < 1024 || addr > 60000)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return INVALID_ADRESS;
    }

    LocalService.sin_family = AF_INET;
    LocalService.sin_port = htons(LOCALADRESS);

    mySocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return SOCKET_ERROR;
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
        return INVALID_ADRESS;
    }

    // Check if destination port is within our desired interval limits
    if(dst < 100 || dst > 60000)
    {
        printf("%s","Not a valid port. Did you enter the addres of your butt you dum liberal?");
        return INVALID_ADRESS;
    }

    remoteService.sin_family = AF_INET;
    remoteService.sin_port = (in_port_t) dst;

    int connection = connect(mySocket,(struct sockaddr *)&remoteService,sizeof (remoteService));
    if(connection < 0)
        return CONNECTION_ERROR;

    return (int) send(mySocket,data,(uint) len,0);
}

int radio_recv(int *src, char *data, int to_ms)
{
    /*
     * TODO: Need to implement some error functionality
     */

    // Check if adress is valid and initialize struct for later use
    struct sockaddr_in remoteService;
    if(inet_aton(LOCALHOST,&remoteService.sin_addr) == 0 || *src < 100 || *src > 60000)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return INVALID_ADRESS; // INVALID_ADRESS
    }

    remoteService.sin_family = AF_INET;
    remoteService.sin_port = (in_port_t) *src;

    int connection = connect(mySocket,(struct sockaddr *)&remoteService,sizeof (remoteService));
    if(connection < 0)
        return CONNECTION_ERROR; // CONNECTION_ERROR


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
            return (int) bytes_recieved;
    }

    return TIMEOUT; // TIMEOUT
}
