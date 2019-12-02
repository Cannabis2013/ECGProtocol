#include "radio.h"

int radio_init(int addr)
{
    // NOTE: Has to find an alternative solution
    unique_adress = permuteQPR((permuteQPR(1) + SEED) ^ 0x5bf03635);

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

    fcntl(mySocket,F_SETFL,O_NONBLOCK);

    return 0;
}

int radio_send(int dst, char *data, int len)
{

    // Initialize the frame
    Frame_PTU frame;

    frame.frame.header.src = htole16(LocalService.sin_port);
    frame.frame.header.dst = (ushort) dst;
    frame.frame.header.lenght = FRAME_PAYLOAD_SIZE;
    frame.frame.unique_adress = unique_adress;

    cp_data(frame.frame.payload,data,FRAME_PAYLOAD_SIZE);

    if(mySocket < 0)
        return SOCKET_ERROR;

    // Check if adress is valid and initialize struct for later use
    struct sockaddr_in remoteService;

    if(inet_aton(LOCALHOST,&remoteService.sin_addr) == 0)
    {
        printf("%s","Not a valid IPv4 adress! Did you enter your household adress you idiot?");
        return INVALID_ADRESS;
    }

    // Check if the port number adress is wihthin the desired port range
    if(dst < 1024 || dst > 65336)
    {
        printf("%s","Not a valid port. Did you enter the addres of your butt you dum liberal?");
        return INVALID_ADRESS;
    }

    remoteService.sin_family = AF_INET;
    remoteService.sin_port = htons((uint16_t)dst);

    int connection = connect(mySocket,(struct sockaddr *)&remoteService,sizeof (remoteService));
    if(connection < 0)
        return CONNECTION_ERROR;
    int bytes_send = (int) send(mySocket,frame.raw,(uint) len,0);
    block(950); // Assuming the above operations took about 50ms

    return bytes_send;
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

    /*
     * Connect the socket
     *
     * Note: Im not sure whether adress structure should contain the listening port
     *  or is just a temporary container for the peer adress to be stored.
     */

    int connection = connect(mySocket,(struct sockaddr *)&LocalService,sizeof (LocalService));
    if(connection < 0)
        return CONNECTION_ERROR; // CONNECTION_ERROR

    if(to_ms == 0)
    {
        ssize_t bytes_recieved = recv(mySocket,data,FRAME_PAYLOAD_SIZE,0);
        if(bytes_recieved <= 0)
            return TIMEOUT;
        else
            return (int) bytes_recieved;
    }

    Frame_PTU recieved_frame;

    start_timer();
    while (time_elapsed() <= to_ms || to_ms < 0) {
        ssize_t bytes_recieved = recv(mySocket,recieved_frame.raw,FRAME_PAYLOAD_SIZE,0);
        if(bytes_recieved > 0)
        {
            block(950);
            return (int) bytes_recieved;
        }
    }

    *src = htobe16(recieved_frame.frame.header.src);
    cp_data(data,recieved_frame.frame.payload,FRAME_PAYLOAD_SIZE);

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
