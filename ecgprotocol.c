#include "ecgprotocol.h"


int ecg_send(int dst, char *data, int len,int to_ms)
{
    // INITIAL STATE
    // Establish connection and exchange information between sender and reciever (handshake)
    if(!channel_established)
    {
        Packet initial_packet;
        Header initial_header;
        initial_header.dst = (ushort) dst;
        initial_header.src = (ushort) LOCALADRESS;
        initial_header.type.type = INIT;

        initial_packet.header = initial_header;

        // Transmit the packet. Try to do so while obtained results code indicate errors.

        if(try_send(&initial_packet,dst,4,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        /*
         * Await and recieve reply from remote of a total of 4 times. This is a blocking call
         */

        Packet recieved_packet;
        if(await_reply(&recieved_packet,dst,to_ms,4,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        if(recieved_packet.header.type.type == ACKWM)
        {
            remote.unique_adrs = recieved_packet.header.magic_key;
            remote.ip_byte_adrs = recieved_packet.header.src;
        }
        channel_established = 0;
    }

    int str_index = 0;
    int residue_data_len = len;
    while (residue_data_len > 0)
    {
        Packet packet;
        Data d;
        d.type.type = DATA;
        int packet_len = residue_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : residue_data_len;
        cp_data(d.data,data + str_index,packet_len);
        packet.data = d;
        if(try_send(&packet,remote.ip_byte_adrs,4,FRAME_PAYLOAD_SIZE) <0)
            return error.error_code;

        // Wait for reply to esnure data has arrived at its destination safely
        Packet reply;
        if(await_reply(&reply,0,to_ms,4,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        if(reply.header.type.type == P_ACKWM)
        {
            residue_data_len -= FRAME_PAYLOAD_SIZE;
            str_index += residue_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : residue_data_len;
        }
    }
    Packet packet_complete;
    Header final_transmission_header;
    final_transmission_header.src = (ushort) LOCALADRESS;
    final_transmission_header.dst = remote.ip_byte_adrs;
    final_transmission_header.type.type = END_OF_PACKET;
    packet_complete.header = final_transmission_header;

    /* Reaching this state we have to assume the whole pack has succesfully been transmitted.
     * But what about the scenario the final transmission, indicating a succesfull packet transmission, fails?
     */

    if(try_send(&packet_complete,remote.ip_byte_adrs,8,FRAME_PAYLOAD_SIZE) <0)
        return error.error_code;

    Packet recieved_packet;
    if(await_reply(&recieved_packet,remote.ip_byte_adrs,to_ms,4,FRAME_PAYLOAD_SIZE) < 0)
        return error.error_code;

    return len;
}

int ecg_recieve(int src, char *data,int len, int to_ms)
{

    /*
     * Note: Use STRCAT function to append an array to another array
     */
    Packet _packet;

    memcpy(&_packet.raw,data,FRAME_PAYLOAD_SIZE);
}

void verifyChecksum()
{

}

int ecg_init(int addr)
{
    int status = radio_init(addr);
    if(status == INVALID_ADRESS)
    {

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
