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

int ecg_send(int dst, char *packet, int len,int to_ms)
{
    /*
     * Statistics
     *  - Session time should be identical to transmission time for the client
     */
    session_statistics.packets_lost = 0;

    /*
     * INITIAL STATE
     *  - Establish connection and exchange information between sender and reciever (handshake)
     */

    Packet initial_packet;
    initial_packet.header.type.type = INIT;

    TRANSMIT_DETAILS t_details;
    if(send_and_await_reply(&initial_packet,dst,CONNECTION_SEND_ATTEMPT,to_ms,AWAIT_TIMEOUT,&t_details) < 0)
        return error.error_code;

    if(t_details.p_recv.header.type.type != ACK)
    {
        error.error_code = CONNECTION_HANDSHAKE_FAILED;
        cp_data(error.error_description,
                "Handshake failed. Failed to establish connection..",
                sizeof ("Handshake failed. Failed to establish connection."));
        return error.error_code;
    }

    remote.connection_established = 1;

    /*
     * "SEND DATA" STATE
     *  - Splits the data into chunks of each size 128b
     */

    int str_index = 0;
    int residual_data_len = len;
    uint total_sent = 0;
    while (residual_data_len > 0)
    {
        Packet chunk_packet;

        chunk_packet.chunk.type.type = CHUNK;
        uint packet_len = residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : (uint) residual_data_len;
        chunk_packet.chunk.size = packet_len;
        chunk_packet.chunk.t_send = total_sent;
        cp_data(chunk_packet.chunk.data,packet + str_index, packet_len);
        chunk_packet.chunk.seal = fingerprint(chunk_packet.chunk.data,KEY1,FRAME_PAYLOAD_SIZE);

        TRANSMIT_DETAILS t_details;
        if(send_and_await_reply(&chunk_packet,dst,CONNECTION_SEND_ATTEMPT,to_ms,AWAIT_TIMEOUT,&t_details) < 0)
            return error.error_code;

        if(t_details.p_recv.header.type.type == P_ACK)
        {
            residual_data_len -= FRAME_PAYLOAD_SIZE;
            int payload_size = residual_data_len >= FRAME_PAYLOAD_SIZE ? FRAME_PAYLOAD_SIZE : residual_data_len;
            str_index += payload_size;
            total_sent = (uint) payload_size;
        }
    }

    Packet packet_complete;
    packet_complete.header.type.type = COMPLETE;

    /*
     * To reduce the risks for a final transmission fail, we increases the number of connection attempts
     * to 8.
     */

    if(try_send(&packet_complete,dst,CONNECTION_FINAL_ATTEMP) < 0)
        return error.error_code;

    remote.connection_established = 0;
    remote.peer_adrs = 0;
    remote.peer_id = 0;


    return len;
}

int ecg_recieve(int src, char *packet,int len, int to_ms)
{
    // Statistics


    // Silence warnings
    VAR_UNUSED(len);
    VAR_UNUSED(src);

    if(mySocket < 0)
        return SOCKET_ERROR;

    uint total_chunk_recieved = 0;

    TIMER_IN time_in;

    // Initiate the loop
    while (time_elapsed(&time_in) <= (long) to_ms ||
           remote.connection_established == 0) {

        Packet recv_packet;
        int bytes_recieved = await_reply(&recv_packet,-1,AWAIT_CONTIGUOUS);
        if(bytes_recieved < 0)
            return error.error_code;

        if(recv_packet.header.type.type == INIT)
        {
            // Statistics
            session_statistics.transmission_start_clock= clock();

            remote.connection_established = 1;

            Packet p_acknowledge;
            p_acknowledge.header.type.type = ACK;

            if(try_send(&p_acknowledge,remote.peer_adrs,CONNECTION_INIT_ATTEMPT) < 0)
                return error.error_code;

            start_timer(&time_in);
        }
        else if(recv_packet.chunk.type.type == CHUNK)
        {
            if(recv_packet.chunk.seal != fingerprint(recv_packet.chunk.data,KEY1,FRAME_PAYLOAD_SIZE))
            {
                printf("Checksum: %d\n",recv_packet.chunk.seal);
                printf("Checksum cmp: %d\n",fingerprint(recv_packet.chunk.data,KEY2,FRAME_PAYLOAD_SIZE));

                Packet p_fail_packet;
                p_fail_packet.header.type.type = P_CHECKSUM_FAIL;
                if(!try_send(&p_fail_packet,remote.peer_adrs,CONNECTION_INIT_ATTEMPT))
                    return error.error_code;
            }
            else
            {
                uint indice_begin = total_chunk_recieved;
                uint chunk_size = recv_packet.chunk.size;
                cp_data(packet + indice_begin,recv_packet.chunk.data,chunk_size);
                total_chunk_recieved += chunk_size;

                Packet p_ack_packet;
                p_ack_packet.header.type.type = P_ACK;

                if(try_send(&p_ack_packet,remote.peer_adrs,CONNECTION_SEND_ATTEMPT) < 0)
                    return error.error_code;

            }
        }
        else if(recv_packet.chunk.type.type == RESEND)
        {
            if(recv_packet.chunk.seal != fingerprint(recv_packet.chunk.data,KEY1,FRAME_PAYLOAD_SIZE))
            {
                Packet p_fail_packet;
                p_fail_packet.header.type.type = P_CHECKSUM_FAIL;
                if(!try_send(&p_fail_packet,remote.peer_adrs,CONNECTION_INIT_ATTEMPT))
                    return error.error_code;
            }
            else
            {
                uint indice_begin = recv_packet.chunk.t_send;
                uint chunk_size = recv_packet.chunk.size;
                cp_data(packet + indice_begin,recv_packet.chunk.data,chunk_size);
                total_chunk_recieved += indice_begin + chunk_size;
                int p_lost = ++session_statistics.packets_lost;
                session_statistics.packets_lost = ++p_lost;

                Packet p_ack_packet;
                p_ack_packet.header.type.type = P_ACK;

                if(try_send(&p_ack_packet,remote.peer_adrs,CONNECTION_SEND_ATTEMPT) < 0)
                    return error.error_code;
            }
        }
        else if (recv_packet.header.type.type == COMPLETE) {
            session_statistics.peer_adrs = remote.peer_adrs;

            remote.connection_established = 0;
            remote.peer_id = 0;
            remote.peer_adrs = 0;
            session_statistics.transmission_end_clock = clock();

            return (int) total_chunk_recieved;
        }
    }
    return TIMEOUT;
}

int ecg_init(int addr)
{
    session_statistics.session_start_clock = clock();
    remote.connection_established = 0;
    remote.peer_id = 0;
    remote.peer_adrs = 0;

    return radio_init(addr);
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
        else if(status == CONNECTION_REQUEST_IGNORED)
        {
            error.error_code = CONNECTION_REQUEST_IGNORED;
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
        else if(bytes_send == CONNECTION_ERROR && connection_attempts-- < 0)
            return CONNECTION_ERROR;
    }

    return bytes_send;
}



int fingerprint(char *msg, int key,int len)
{
    char sum = 0;
    char *s;
    int a = 0;
    for (s = msg;*s!= 0 && a < len;s++,a++) {
        sum ^= *s;
    }
    return ((int) sum)^key;
}

int send_and_await_reply(Packet *packet,
                         int adrs_reciever,
                         int connection_attempts,
                         int timeout,
                         int mode, TRANSMIT_DETAILS *t)
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
        else if(t->bytes_recv == TIMEOUT && packet->chunk.type.type == CHUNK)
        {
            /*
             * In case network interface fails to either send data or reply.
             */


            packet->chunk.type.type = RESEND;
            error.error_code = t->bytes_recv;
            attempt--;
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

void printArray(char *arr, int len)
{
    for (int i = 0; i < len; ++i) {
        char c = *(arr + i);
        printf("%c",c);
    }
    printf("\n");
}
