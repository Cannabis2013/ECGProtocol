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
    // Initialize the frame
    Frame_PTU ptu;

    ptu.frame.header.src = htole16(LocalService.sin_port);
    ptu.frame.header.dst = (ushort) dst;
    ptu.frame.header.lenght = FRAME_PAYLOAD_SIZE; // Size of raw data
    ptu.frame.unique_adress = unique_adress;

    cp_data(ptu.frame.payload,data,(uint) len);

    if(mySocket < 0)
    {
        uint size_of_msg = sizeof ("Socket not initialized. Please call radio_init() before calling radio_send().");
        cp_data(radio_error.err_msg,"Socket not initialized. Please call radio_init() before calling radio_send().",size_of_msg);
        radio_error.code = SOCKET_ERROR;
        return radio_error.code;
    }

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

    block(950); // Assuming the above operations took about 50ms
    int bytes_send = (int) send(mySocket,ptu.raw,(uint) FRAME_SIZE,0);

    return bytes_send;
}

int radio_recv(int *src, char *data, int to_ms)
{
    Frame_PTU recieved_frame;

    if(to_ms == 0)
    {
        ssize_t bytes_recieved = recv(mySocket,recieved_frame.raw,FRAME_SIZE,MSG_DONTWAIT);

        uint magic_key = recieved_frame.frame.unique_adress;

        if(bytes_recieved > 0)
        {
            if(remote.channel_established == 1 && remote.peer_id != magic_key)
                return INBOUND_REQUEST_IGNORED;
            *src = htobe16(recieved_frame.frame.header.src);
            remote.peer_id = magic_key;
            block(1000);
            strcpy(data,recieved_frame.frame.payload);
            //cp_data(data,recieved_frame.frame.payload,CHUNK_SIZE);
        }
        return (int) bytes_recieved;
    }

    TIMER_IN t_in;
    start_timer(&t_in);
    unsigned long long t_sec =  0;
    while ((t_sec = time_elapsed(&t_in)) <= (unsigned long long) to_ms || to_ms < 0) {
        ssize_t bytes_recieved = recv(mySocket,recieved_frame.raw,FRAME_SIZE,MSG_DONTWAIT);

        uint magic_key = recieved_frame.frame.unique_adress;

        if(bytes_recieved > 0)
        {
            if(remote.channel_established == 1 && remote.peer_id != magic_key)
                return INBOUND_REQUEST_IGNORED;
            remote.peer_id = magic_key;
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
    for (uint i = 0; i < src_len; ++i)
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
