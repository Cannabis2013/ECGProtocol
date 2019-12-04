#include "ecgprotocol.h"

/* We assumes size of data may go beyond the limit of 2400 bits, which is the maximum transfer rate given in this assignment.
 * We divide data into chunks of each 140 bytes (data + additional meta overhead)
 * Our protocol resembles to a great extend the behaviour found in the TCP/IP protocol with respect to the stability and reliability it offers.
 * This means:
 *      - With every packet send, a reply is required, ensuring data has arrived safely, in order to proceed.
 *      - The connection consists of three states:
 *          > INITIAL STATE : Exchange of meta information; also known as  a 'handshake'
 *          > TRANSFER STATE : Each chunk is fingerprintet with a checksum and send to the reciever
 *          > FINAL CHUNK STATE : This state indicates a whole packet sendt. Of course, this will be signalled to the end point.
 *
 */

int ecg_send(int dst, char *data, int len,int to_ms)
{
    /*
     * INITIAL STATE
     *  - Establish connection and exchange information between sender and reciever (handshake)
     */
    // Initialize packet with meta information
    Packet initial_packet;
    initial_packet.header.type.type = INIT;
    initial_packet.header.src = (ushort) htobe16(LocalService.sin_port);
    initial_packet.header.dst = (ushort) dst;
    initial_packet.header.total_size = (uint) len;
    initial_packet.header.protocol = IPPROTO_UDP;

    int attempts = CONNECTION_INIT_ATTEMPT;

    // Establish handshake. Try this in 4 attempts
    while(remote.channel_established == 0)
    {
        if(attempts < 0)
            return CONNECTION_ERROR;

        TRANSMIT_DETAILS t_details;
        if(send_and_await_reply(&initial_packet,dst,CONNECTION_SEND_ATTEMPT,to_ms,AWAIT_TIMEOUT,&t_details) < 0)
            return error.error_code;

        if(t_details.p_recv.header.type.type == ACK)
        {
            remote.peer_adrs = t_details.p_recv.header.src;
            remote.channel_established = 1;
        }
        attempts--;
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

        packet.chunk.type.type = CHUNK;
        uint packet_len = residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : (uint) residual_data_len;
        cp_data(packet.chunk.data,data + str_index, packet_len);
        packet.chunk.checksum = generateChecksum(packet.chunk.data,KEY1);
        packet.chunk.chunk_size = packet_len;

        TRANSMIT_DETAILS t_details;
        if(send_and_await_reply(&packet,dst,CONNECTION_SEND_ATTEMPT,to_ms,AWAIT_TIMEOUT,&t_details) < 0)
            return error.error_code;

        if(t_details.p_recv.header.type.type == P_ACK)
        {
            residual_data_len -= FRAME_PAYLOAD_SIZE;
            str_index += residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : residual_data_len;
        }
    }


    Packet packet_complete;
    Header final_transmission_header;
    final_transmission_header.src = (ushort) LOCALADRESS;
    final_transmission_header.dst = remote.peer_adrs;
    final_transmission_header.type.type = COMPLETE;
    packet_complete.header = final_transmission_header;

    /*
     * To reduce the risks for a final transmission fail, we increases the number of connection attempts
     * to 8.
     */

    if(try_send(&packet_complete,dst,CONNECTION_FINAL_ATTEMP) < 0)
        return error.error_code;

    remote.channel_established = 0;
    remote.peer_adrs = 0;
    remote.peer_id = 0;

    return len;
}

int ecg_recieve(int src, char *data,int len, int to_ms)
{
    VAR_UNUSED(len);
    VAR_UNUSED(src);

    if(mySocket < 0)
        return SOCKET_ERROR;

    uint total_chunk_recieved = 0;

    TIMER_IN time_in;

    // Initiate the loop
    while (1) {

        if(time_elapsed(&time_in) > (unsigned long long) to_ms && remote.channel_established == 1)
            return TIMEOUT;

        Packet recieved_packet;
        int bytes_recieved = await_reply(&recieved_packet,-1,AWAIT_CONTIGUOUS);
        if(bytes_recieved < 0)
            return error.error_code;


        if(recieved_packet.header.type.type == INIT)
        {
            // Saving
            remote.peer_adrs = recieved_packet.header.src;
            remote.channel_established = 1;

            uint total_pending_size = recieved_packet.header.total_size;

            Packet p_ack;
            p_ack.header.type.type = ACK;
            p_ack.header.src = (ushort) htobe16(LocalService.sin_port);
            p_ack.header.dst = remote.peer_adrs;

            if(try_send(&p_ack,remote.peer_adrs,CONNECTION_INIT_ATTEMPT) < 0)
                return error.error_code;

            start_timer(&time_in);
        }
        else if(recieved_packet.chunk.type.type == CHUNK)
        {
            if(recieved_packet.chunk.checksum != generateChecksum(recieved_packet.chunk.data,KEY1))
            {
                Packet p_fail_packet;
                p_fail_packet.header.type.type = P_CHECKSUM_FAIL;
                if(!try_send(&p_fail_packet,remote.peer_adrs,CONNECTION_INIT_ATTEMPT))
                    return error.error_code;
            }
            else
            {
                uint chunk_size = recieved_packet.chunk.chunk_size;
                cp_data(data + total_chunk_recieved,recieved_packet.chunk.data,chunk_size);
                total_chunk_recieved += recieved_packet.chunk.chunk_size;

                Packet p_ack_packet;
                p_ack_packet.header.type.type = P_ACK;

                if(try_send(&p_ack_packet,remote.peer_adrs,CONNECTION_SEND_ATTEMPT) < 0)
                    return error.error_code;
            }
        }
        else if (recieved_packet.header.type.type == COMPLETE) {
            remote.channel_established = 0;
            remote.peer_id = 0;
            remote.peer_adrs = 0;
            break;
        }
    }

    return 0;
}

int ecg_init(int addr)
{
    remote.channel_established = 0;
    remote.peer_id = 0;
    remote.peer_adrs = 0;

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

int await_reply(Packet *buffer,int timeout, int mode)
{
    int status = 0, adrs_from;
    while((status = radio_recv(&adrs_from,buffer->raw,timeout)) < 0)
    {
        if(status == INVALID_ADRESS)
        {
            error.error_code = INVALID_ADRESS;
            strcpy(error.error_description,"Invalid adress provided.");
            return INVALID_ADRESS;
        }
        else if(status == TIMEOUT && mode  == AWAIT_TIMEOUT)
        {
            error.error_code = TIMEOUT;
            strcpy(error.error_description,"Connection timed out. Remote is probably offline.");

            return TIMEOUT;
        }
        else if(status == INBOUND_REQUEST_IGNORED)
        {
            error.error_code = INBOUND_REQUEST_IGNORED;
            return error.error_code;
        }
    }

    return status;
}

int try_send(Packet *packet,int adrs_reciever, int connection_attempts)
{
    int bytes_send = 0;

    while((bytes_send = radio_send(adrs_reciever,packet->raw,CHUNK_SIZE)) < 0)
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

ushort generateChecksum(char *msg, ushort key)
{
    char sum = 0;
    char *s;
    for (s = msg;*s!= 0;s++) {
        sum ^= *s;
    }
    return ((ushort) sum)^key;
}

int send_and_await_reply(Packet *packet, int adrs_reciever, int connection_attempts, int timeout, int mode, TRANSMIT_DETAILS *t)
{
    int attempt = connection_attempts;
    while(attempt > 0)
    {
        if((t->bytes_sent= try_send(packet,adrs_reciever,connection_attempts)) < 0)
            return error.error_code;

        Packet p_recv;
        if((t->bytes_recv = await_reply(&p_recv,timeout,mode)) >= 0)
        {
            t->p_recv = p_recv;
            return 0;
        }
        else if(t->bytes_recv == TIMEOUT)
        {
            error.error_code = t->bytes_recv;
            attempt--;
        }
        else
            return error.error_code;
    }

    return error.error_code;
}
