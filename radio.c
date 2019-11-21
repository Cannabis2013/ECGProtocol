#include "radio.h"

int radio_init(int addr)
{
    int mySocket;

    struct sockaddr_in ClientService;
    struct in_addr DummyObject;

    ClientService.sin_family = AF_INET;
    ClientService.sin_port = htons(3377);
    ClientService.sin_addr.s_addr = (uint) inet_aton(RECIEVER_ADRS,&DummyObject);

    mySocket = socket(AF_INET,SOCK_STREAM,IPPROTO_UDP);

    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return -1;
    }

    int connection = connect(mySocket,(struct sockaddr *)&ClientService,sizeof (ClientService));

    return 0;
}
