#include "ecgprotocol.h"


int ecg_send(int dst, char *data, int len,int to_ms)
{
    /*
     * INITIAL STATE
     *  - Establish connection and exchange information between sender and reciever (handshake)
     */

    if(!channel_established)
    {
        Packet initial_packet;
        Header initial_header;
        initial_header.dst = (ushort) dst;
        initial_header.src = (ushort) LOCALADRESS;
        initial_header.type.type = INIT;

        initial_packet.header = initial_header;

        // Transmit the packet. Try to do so while obtained result codes indicates error.

        if(try_send(&initial_packet,dst,CONNECTION_SEND_ATTEMPT,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        /*
         * Await and recieve reply from remote of a total of 4 times. This is a blocking call
         */

        Packet recieved_packet;
        if(await_reply(&recieved_packet,dst,to_ms,CONNECTION_AWAIT_ATTEMPT,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        if(recieved_packet.header.type.type == ACKWM)
        {
            remote.unique_adrs = recieved_packet.header.magic_key;
            remote.ip_byte_adrs = recieved_packet.header.src;
        }
        channel_established = 0;
    }

    /*
     * "SEND DATA" STATE
     *  - Splits the data into chunks of each size 128b
     */

    int str_index = 0;
    int residual_data_len = len;
    while (residual_data_len > 0)
    {
        Packet packet;
        Data d;
        d.type.type = CHUNK;
        int packet_len = residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : residual_data_len;
        cp_data(d.data,data + str_index,packet_len);
        packet.data = d;

        // NOTE: Still need to implement integrity verification

        if(try_send(&packet,remote.ip_byte_adrs,CONNECTION_SEND_ATTEMPT,FRAME_PAYLOAD_SIZE) <0)
            return error.error_code;

        // Wait for reply to esnure data has arrived at its destination safely
        Packet reply;
        if(await_reply(&reply,0,to_ms,CONNECTION_AWAIT_ATTEMPT,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        if(reply.header.type.type == P_ACKWM)
        {
            residual_data_len -= FRAME_PAYLOAD_SIZE;
            str_index += residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : residual_data_len;
        }
    }
    Packet packet_complete;
    Header final_transmission_header;
    final_transmission_header.src = (ushort) LOCALADRESS;
    final_transmission_header.dst = remote.ip_byte_adrs;
    final_transmission_header.type.type = LAST_CHUNK;
    packet_complete.header = final_transmission_header;

    /* Reaching this state we have to assume the whole pack has succesfully been transmitted.
     * To reduce the risks for a final transmission fail, we set the number of connection attempts
     * at both sending and recieving to 8.
     */

    if(try_send(&packet_complete,remote.ip_byte_adrs,CONNECTION_FINAL_ATTEMP,FRAME_PAYLOAD_SIZE) <0)
        return error.error_code;

    Packet recieved_packet;
    if(await_reply(&recieved_packet,remote.ip_byte_adrs,to_ms,CONNECTION_FINAL_ATTEMP,FRAME_PAYLOAD_SIZE) < 0)
        return error.error_code;

    return len;
}

int ecg_recieve(int src, char *data,int len, int to_ms)
{
    /*
     * Note: Use STRCAT function to append an array to another array
     */

    return 0;
}

void verifyChecksum()
{

}

int ecg_init(int addr)
{
    int status = radio_init(addr);
    if(status == INVALID_ADRESS)
    {
        printf("Adress not valid. No reason to proceed.");
        exit(-1);
    }
    else if (status == CONNECTION_ERROR) {

    }
}

void cp_data(char*dst, char*src, int src_len)
{
    for (int i = 0; i < src_len; ++i)
        *(dst + i) = *(src + i);
}

int await_reply(Packet *buffer,int adrs_from,int timeout,int connection_attempts, int len)
{
    if(adrs_from)
        adrs_from = remote.ip_byte_adrs;
    int status = 0;
    while((status = radio_recv(&adrs_from,buffer->raw,timeout))<0)
    {
        if(!connection_attempts)
        {
            error.error_code = CONNECTION_ERROR;
            strcpy(error.error_description,"Connection lost after 4 attemps. Cancelling transmission.");

            return CONNECTION_ERROR;
        }

        if(status == CONNECTION_ERROR)
            connection_attempts--;
        else if(status == TIMEOUT)
        {
            error.error_code = TIMEOUT;
            strcpy(error.error_description,"Connection timed out. Remote is probably offline.");

            return TIMEOUT;
        }
    }

    return status;
}

int try_send(Packet *packet,int adrs_reciever, int connection_attempts, int len)
{
    int status = 0;
    while((status = radio_send(adrs_reciever,packet->raw,len) < 0))
    {
        if(status == INVALID_ADRESS)
        {
            error.error_code = INVALID_ADRESS;
            strcpy(error.error_description,"The adress is not a valid IPv4 adress or port out of range");

            return INVALID_ADRESS;
        }
        else if(status == CONNECTION_ERROR)
        {
            connection_attempts--;
        }
        if(connection_attempts < 0)
            return CONNECTION_ERROR;
    }

    return status;
}
