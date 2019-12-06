#include "radio.h"

int radio_init(int addr)
{
    // NOTE: Has to find an alternative solution

    unique_adress = permuteQPR((permuteQPR(1) + SEED)^0x5bf03635);

    remote.connection_established = 0;

    if(inet_aton(LOCALHOST,&LocalService.sin_addr) == 0 || addr < 1024 || addr > 65336)
    {
        cp_data(radio_error.err_msg,"Invalid IPv4 adress.",sizeof ("Invalid IPv4 adress."));
        return radio_error.code = INVALID_ADRESS;
    }
    LocalService.sin_family = AF_INET;
    LocalService.sin_port = htons((uint16_t)addr);

    mySocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(mySocket < 0)
    {
        printf("Socket is invalid");
        cp_data(radio_error.err_msg,"Failed to initialize socket.",sizeof ("Failed to initialize socket."));
        return radio_error.code = SOCKET_ERROR;
    }

    /*
     * Sets the socket in listening state
     */

    if(bind(mySocket,(struct sockaddr *)&LocalService,sizeof (LocalService)) == -1)
    {
        cp_data(radio_error.err_msg,"Failed to bind socket.",sizeof ("Failed to bind socket."));
        return radio_error.code = CONNECTION_ERROR;
    }

    return 0;
}

int radio_send(int dst, char *data, int len)
{
    /*
     * Ensuring init() has been called before this function call
     * Returns appropriate error code if necessary
     */

    if(mySocket < 0)
    {
        uint size_of_msg = sizeof ("Socket not initialized. Please call radio_init() before calling radio_send().");
        cp_data(radio_error.err_msg,
                "Socket not initialized. Please call radio_init() before calling radio_send().",size_of_msg);
        return radio_error.code = SOCKET_ERROR;
    }

    // Initialize the frame
    Frame_PDU pdu;

    pdu.frame.header.src = htobe16(LocalService.sin_port);
    pdu.frame.header.dst = (ushort) dst;
    pdu.frame.header.lenght = FRAME_PAYLOAD_SIZE; // Size of raw data
    pdu.frame.unique_adress = unique_adress;
    cp_data(pdu.frame.payload,data,(uint) len);

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

    /* Establish connection
     * NOTE: This is not the common UDP way of transmitting data;
     * but it provides us with a quick opportunity to check whether source or destination is offline.
     */

    int connection = connect(mySocket,(struct sockaddr *)&remoteService,sizeof (remoteService));
    if(connection < 0)
        return CONNECTION_ERROR;

    block(SLEEP_TIME); // Assuming the above operations took about 50ms
    int bytes_send = (int) send(mySocket,pdu.raw,(uint) FRAME_SIZE,0);

    return bytes_send;
}

int radio_recv(int *src, char *data, int to_ms)
{
    VAR_UNUSED(src);

    // Ensuring init() has been called before this function is called
    if(mySocket < 0)
    {
        uint size_of_msg = sizeof ("Socket not initialized. Please call radio_init() before calling radio_send().");
        cp_data(radio_error.err_msg,"Socket not initialized. Please call radio_init() before calling radio_send().",size_of_msg);
        radio_error.code = SOCKET_ERROR;
        return radio_error.code;
    }

    Frame_PDU pdu;

    if(to_ms == 0)
    {
        ssize_t bytes_recieved = recv(mySocket,pdu.raw,FRAME_SIZE,MSG_DONTWAIT);

        uint magic_key = pdu.frame.unique_adress;

        if(bytes_recieved > 0)
        {
            if(remote.connection_established == 1 && remote.peer_id != magic_key)
                return CONNECTION_REQUEST_IGNORED;

            if(pdu.frame.header.lenght > FRAME_PAYLOAD_SIZE)
                return CONNECTION_REQUEST_IGNORED;

            *src = pdu.frame.header.src;
            remote.peer_adrs = pdu.frame.header.src;
            remote.peer_id = magic_key;
            block(SLEEP_TIME);
            cp_data(data,pdu.frame.payload,CHUNK_SIZE);
        }
        return (int) bytes_recieved;
    }

    TIMER_IN t_in;
    start_timer(&t_in);
    while (time_elapsed(&t_in) <= (long) to_ms || to_ms < 0) {
        ssize_t bytes_recieved = recv(mySocket,pdu.raw,FRAME_SIZE,MSG_DONTWAIT);

        uint magic_key = pdu.frame.unique_adress;

        if(bytes_recieved > 0)
        {
            if(remote.connection_established == 1 && remote.peer_id != magic_key)
                return CONNECTION_REQUEST_IGNORED;

            if(pdu.frame.header.lenght > FRAME_PAYLOAD_SIZE)
                return CONNECTION_REQUEST_IGNORED;

            remote.peer_id = magic_key;
            remote.peer_adrs = pdu.frame.header.src;
            cp_data(data,pdu.frame.payload,CHUNK_SIZE);
            block(SLEEP_TIME);
            return (int) bytes_recieved;
        }
    }

    return TIMEOUT; // TIMEOUT
}

/* This algorithm is found on the internet.
 * Credit goes solely to the author Jeff Preshing who presented this algorithm on his website Preshing.com.
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
    int n = (number == 0) ? 0 : log10(number);

    char *result = malloc(sizeof (char*));

    for (int i = n; i >= 0; --i, number /=10)
        *(result + i) = number % 10 + 48;

    return result;
}
