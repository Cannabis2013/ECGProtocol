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

void print_array(char *arr,int len)
{
    for (int i = 0; i < len; ++i) {
        char c = *(arr + i);
        printf("%c",c);
    }
}

int main(int argc, char *argv[])
{
    char msg[4096];

    int msg_size = sizeof ("When a client hasn't received any reply from its destination while in DATA TRANSFER STATE, due to the pre-mentioned reasons, it will enter a new state trying to send the packet again. The packet this time has its type initialized to \\code{RESEND} so the receiver know it's the same packet, and not new data. The client does this by transmitting the total raw payload size sent, which is updated for every time it receives a reply packet with its type initialized to \code{P\_ACK}. In this way it holds track of how many bytes of data the listener device actually have received, and the receiver likewise uses this field as an index to concatenate its data array with the data chunk received from the client.");;

    ushort src_port = 35000;
    ushort dst_port = 22500;
    int timeout = 2500;
    int role = CLIENT_ROLE;


    char data[msg_size];
    cp_data(data,"When a client hasn't received any reply from its destination while in DATA TRANSFER STATE, due to the pre-mentioned reasons, it will enter a new state trying to send the packet again. The packet this time has its type initialized to \\code{RESEND} so the receiver know it's the same packet, and not new data. The client does this by transmitting the total raw payload size sent, which is updated for every time it receives a reply packet with its type initialized to \code{P\_ACK}. In this way it holds track of how many bytes of data the listener device actually have received, and the receiver likewise uses this field as an index to concatenate its data array with the data chunk received from the client.",msg_size);


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
           msg_size = (ulong) strtol(argv[i+2],NULL,10);
           cp_data(msg,argv[i+1],(uint)msg_size);
           i = (i < argc) ? ++i : i;
       }
    }

    printf("Initial state:\n");
    printf("\tSource port: %d\n",src_port);
    printf("\tDestination port: %d\n",dst_port);
    printf("\tData out: %s\n",msg);
    printf("\tTimeout: %d\n",timeout);
    printf("\tTransmission size: %d\n",msg_size);

    char outbound_data[msg_size];
    cp_data(outbound_data,data,(uint) msg_size);

    // Initialize the radio chip
    if(ecg_init((int) src_port)<0)
        return -1;

    TIMER_IN time_str;
    start_timer(&time_str);

    if(role == SERVER_ROLE)
    {
        printf("\n\nNow listens on port: %d\n",src_port);
        while (1) {
            char inbound_data[4096];
            int bytes_recieved = ecg_recieve(0,inbound_data,0,timeout);
            if(bytes_recieved < 0)
                return -1;
            session_statistics.session_end_clock = clock();

            printf("\nStatistics\n");
            printf("################\n");
            printf("Source port: %d\n",session_statistics.peer_adrs);
            printf("Bytes recieved: %d b\n",bytes_recieved);
            printf("Packets lost: %d\n",session_statistics.packets_lost);
            printf("Message: %s\n",inbound_data);
            printf("Session duration: %ld ms\n",toMs(session_statistics.session_end_clock - session_statistics.session_start_clock));
            printf("Latency: %ld ms\n",toMs(session_statistics.transmission_end_clock - session_statistics.transmission_start_clock));

            bzero(inbound_data,4096);
        }

        return 0;
    }

    int bytes_sent = ecg_send(dst_port,outbound_data,msg_size,timeout);

    session_statistics.session_end_clock = clock();

    printf("\nStatistics\n");
    printf("################\n");
    printf("Bytes sent: %d b \n",bytes_sent);
    printf("Packets lost: %d\n",session_statistics.packets_lost);
    printf("Session duration: %ld ms\n",toMs(session_statistics.session_end_clock - session_statistics.session_start_clock));
    return 0;
}
