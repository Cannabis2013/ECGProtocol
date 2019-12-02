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
        initial_packet.header.type.type = INIT;
        initial_packet.header.src = (ushort) htobe16(LocalService.sin_port);
        initial_packet.header.dst = (ushort) dst;
        initial_packet.header.total_size = (uint) len;
        initial_packet.header.magic_key = unique_adress;
        initial_packet.header.protocol = IPPROTO_UDP;

        // Transmit the packet. Try to do so while obtained result codes indicates error.

        if(try_send(&initial_packet,dst,CONNECTION_SEND_ATTEMPT,TOTAL_PAYLOAD_SIZE) < 0)
            return error.error_code;

        /*
         * Await and recieve reply from remote of a total of 4 times. This is a blocking call
         */

        Packet recieved_packet;
        if(await_reply(&recieved_packet,to_ms,CONNECTION_AWAIT_ATTEMPT,TOTAL_PAYLOAD_SIZE) < 0)
            return error.error_code;

        if(recieved_packet.header.type.type == ACK)
        {
            remote.unique_adrs = recieved_packet.header.magic_key;
            remote.ip_byte_adrs = recieved_packet.header.src;
        }
        channel_established = 1;
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
        Chunk d;
        d.type.type = CHUNK;
        uint packet_len = residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : (uint) residual_data_len;
        cp_data(d.data,data + str_index,(int) packet_len);
        packet.chunk = d;
        packet.chunk.chunk_size = packet_len;

        // NOTE: Still need to implement integrity verification

        if(try_send(&packet,remote.ip_byte_adrs,CONNECTION_SEND_ATTEMPT,TOTAL_PAYLOAD_SIZE) <0)
            return error.error_code;

        // Wait for reply to esnure data has arrived at its destination safely
        Packet reply;
        if(await_reply(&reply,to_ms,CONNECTION_AWAIT_ATTEMPT,FRAME_PAYLOAD_SIZE) < 0)
            return error.error_code;

        if(reply.header.type.type == P_ACK)
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
     * To reduce the risks for a final transmission fail, we increases the number of connection attempts
     * to 8.
     */

    if(try_send(&packet_complete,remote.ip_byte_adrs,CONNECTION_FINAL_ATTEMP,FRAME_PAYLOAD_SIZE) <0)
        return error.error_code;

    Packet recieved_packet;
    if(await_reply(&recieved_packet,to_ms,CONNECTION_FINAL_ATTEMP,FRAME_PAYLOAD_SIZE) < 0)
        return error.error_code;

    channel_established = 0;
    return len;
}

int ecg_recieve(int src, char *data,int len, int to_ms)
{
    char* accumulated_data = NULL;
    uint total_chunk_recieved = 0;

    while (1) {
        Packet recieved_packet;

        if(!await_reply(&recieved_packet,to_ms,CONNECTION_LISTEN_ATTEMPT,FRAME_PAYLOAD_SIZE))
            return error.error_code;

        if(recieved_packet.header.type.type == INIT)
        {
            remote.unique_adrs = recieved_packet.header.magic_key;
            remote.ip_byte_adrs = recieved_packet.header.src;
            uint total_pending_size = recieved_packet.header.total_size;
            accumulated_data = malloc(total_pending_size);

            Packet ACK_PACKET;
            ACK_PACKET.header.type.type = ACK;
            ACK_PACKET.header.src = (ushort) htobe16(LocalService.sin_port);
            ACK_PACKET.header.dst = (ushort) src;
            ACK_PACKET.header.magic_key = unique_adress;

            if(!try_send(&ACK_PACKET,remote.ip_byte_adrs,CONNECTION_INIT_ATTEMPT,FRAME_PAYLOAD_SIZE))
                return error.error_code;
        }
        else if(recieved_packet.chunk.type.type == CHUNK)
        {
            // TODO: Again, we have to implement some kind of integrety verification.
            uint chunk_size = recieved_packet.chunk.chunk_size;
            cp_data(accumulated_data + total_chunk_recieved,recieved_packet.chunk.data,chunk_size);
            total_chunk_recieved += recieved_packet.chunk.chunk_size;
        }

        else if (recieved_packet.header.type.type == LAST_CHUNK) {
            cp_data(data,accumulated_data,total_chunk_recieved);
        }
    }

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
        exit(-1);
    }

    return status;
}

int await_reply(Packet *buffer,int timeout,int connection_attempts, int len)
{
    int status = 0, adrs_from;
    while((status = radio_recv(&adrs_from,buffer->raw,timeout)) < 0)
    {
        if(connection_attempts == 0)
        {
            error.error_code = CONNECTION_ERROR;
            strcpy(error.error_description,"Connection lost after 4 attemps. Cancelling transmission.");

            return CONNECTION_ERROR;
        }

        if(status == INVALID_ADRESS)
        {
            error.error_code = INVALID_ADRESS;
            strcpy(error.error_description,"Invalid adress provided.");
            return INVALID_ADRESS;
        }
        else if(status == CONNECTION_ERROR)
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
    int bytes_send = 0;

    while((bytes_send = radio_send(adrs_reciever,packet->raw,len)) < 0)
    {
        if(bytes_send == INVALID_ADRESS)
        {
            error.error_code = INVALID_ADRESS;
            strcpy(error.error_description,"The adress is not a valid IPv4 adress or port out of range");

            return INVALID_ADRESS;
        }
        else if(bytes_send == CONNECTION_ERROR)
        {
            connection_attempts--;
        }
        if(connection_attempts < 0)
            return CONNECTION_ERROR;
    }

    return bytes_send;
}
