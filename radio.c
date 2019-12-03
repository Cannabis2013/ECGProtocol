#include "radio.h"

int radio_init(int addr)
{
    // NOTE: Has to find an alternative solution
    unique_adress = permuteQPR((permuteQPR(1) + SEED) ^ 0x5bf03635);

    remote.channel_established = 0;

    if(inet_aton(LOCALHOST,&LocalService.sin_addr) == 0 || addr < 1024 || addr > 65336)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return INVALID_ADRESS;
    }
    LocalService.sin_family = AF_INET;
    LocalService.sin_port = htons((uint16_t)addr);
    mySocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(mySocket < 0)
    {
        printf("Socket is invalid");
        return SOCKET_ERROR;
    }


    if(bind(mySocket,(struct sockaddr *)&LocalService,sizeof (LocalService)) == -1)
    {
        // Error handling
        printf("Shit happens");
        exit(-1);
    }

    //fcntl(mySocket,F_SETFL,O_NONBLOCK);
    return 0;
}

int radio_send(int dst, char *data, int len)
{
    VAR_UNUSED(len);

    // Initialize the frame
    Frame_PTU frame;

    frame.frame.header.src = htole16(LocalService.sin_port);
    frame.frame.header.dst = (ushort) dst;
    frame.frame.header.lenght = FRAME_PAYLOAD_SIZE; // Size of raw data
    frame.frame.unique_adress = unique_adress;

    cp_data(frame.frame.payload,data,CHUNK_SIZE);

    if(mySocket < 0)
        return SOCKET_ERROR;

    // Check if adress is valid and initialize struct for later use
    struct sockaddr_in remoteService;

    if(inet_aton(LOCALHOST,&remoteService.sin_addr) == 0)
    {
        printf("%s","Not a valid IPv4 adress!");
        return INVALID_ADRESS;
    }

    // Check if the port number adress is within the desired port range
    if(dst < 1024 || dst > 65336)
    {
        printf("%s","Not a valid port.");
        return INVALID_ADRESS;
    }

    remoteService.sin_family = AF_INET;
    remoteService.sin_port = htons((uint16_t)dst);

    // Establish connection
    // NOTE: This is not the common UDP way of transmitting data; but it provides us with the opportunity to check whether the connection can be done.
    int connection = connect(mySocket,(struct sockaddr *)&remoteService,sizeof (remoteService));
    if(connection < 0)
        return CONNECTION_ERROR;

    int bytes_send = (int) send(mySocket,frame.raw,(uint) FRAME_SIZE,0);
    block(950); // Assuming the above operations took about 50ms

    return bytes_send;
}

int radio_recv(int *src, char *data, int to_ms)
{
    Frame_PTU recieved_frame;

    if(to_ms == 0)
    {
        ssize_t bytes_recieved = recv(mySocket,recieved_frame.raw,FRAME_SIZE,MSG_DONTWAIT);

        uint magic_key = recieved_frame.frame.unique_adress;
        if(remote.channel_established == 1 && remote.unique_adrs != magic_key)
            return INBOUND_REQUEST_IGNORED;

        if(bytes_recieved > 0)
        {
            *src = htobe16(recieved_frame.frame.header.src);

            cp_data(data,recieved_frame.frame.payload,CHUNK_SIZE);
            block(1000);
        }
        return (int) bytes_recieved;
    }


    start_timer();
    while (time_elapsed() <= to_ms || to_ms < 0) {
        ssize_t bytes_recieved = recv(mySocket,recieved_frame.raw,FRAME_SIZE,MSG_DONTWAIT);

        uint magic_key = recieved_frame.frame.unique_adress;

        if(remote.channel_established == 1 && remote.unique_adrs != magic_key)
            return INBOUND_REQUEST_IGNORED;

        if(bytes_recieved > 0)
        {
            *src = htobe16(recieved_frame.frame.header.src);
            cp_data(data,recieved_frame.frame.payload,CHUNK_SIZE);
            block(950);
            return (int) bytes_recieved;
        }
    }

    return TIMEOUT; // TIMEOUT
}

/* This algorithm is found on the internet.
 * Credit goes solely to the author Jeff Preshing who presented this algorithm on his website Preshing.com with the full url given below:
 * Url: https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
 */

unsigned int permuteQPR(unsigned int x)
{
    static const unsigned int prime = 4294967291;
    if (x >= prime)
        return x;  // The 5 integers out of range are mapped to themselves.
    unsigned int residue = ((unsigned long long) x * x) % prime;
    return (x <= prime / 2) ? residue : prime - residue;
}

void cp_data(char*dst, char*src, uint src_len)
{
    for (int i = 0; i < src_len; ++i)
        *(dst + i) = *(src + i);
}

char* integertoc(uint number)
{
    int n = number == 0 ? 0 : log10(number);

    char *result = malloc(sizeof (char*));

    for (int i = n; i >= 0; --i, number /=10)
        *(result + i) = number % 10 + 48;

    return result;
}
