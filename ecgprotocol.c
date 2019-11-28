#include "ecgprotocol.h"


int ecg_send(int dst, char *data, int len,int to_ms)
{
    int transmission_result = 0;
    Packet intial_packet;

    // INITIAL STATE
    // Establish connection and exchange information between sender and reciever (handshake)
    if(inital_send)
    {
        Header _initial_header;
        _initial_header.dst = (short) dst;
        _initial_header.src = (short) LOCALADRESS;
        _initial_header.type.type = INIT;

        intial_packet.header = _initial_header;

        // Transmit the packet. Try to do so while obtaining results code indicating an error.

        int try_connect = 4;
        while((transmission_result = radio_send(dst,intial_packet.raw,len) < 0))
        {
            if(transmission_result == INVALID_ADRESS)
            {
                error.error_code = INVALID_ADRESS;
                strcpy(error.error_description,"The adress is not a valid IPv4 adress or port out of range");

                return INVALID_ADRESS;
            }
            else if(transmission_result == CONNECTION_ERROR)
            {
                // TODO: Implement some error notification
                try_connect--;
            }
            if(try_connect < 0)
                return CONNECTION_ERROR;
        }

        /*
         * Await and recieve reply from remote of a total of 4 times. This is a blocking call
         */

        Packet recieved_packet;
        int rcv_status = 0;
        int turn = 0;
        while((rcv_status =  radio_recv(&dst,recieved_packet.raw,to_ms)) < 0 || ++turn > 4)
        {
            if(rcv_status == INVALID_ADRESS)
            {
                // TODO: Implement some error notification
                // This cancels the operation
                return INVALID_ADRESS;
            }
            else if(rcv_status == CONNECTION_ERROR)
            {
                // TODO: Implement some error notification
            }
            else if(rcv_status == TIMEOUT)
            {
                // TODO: Implement some error notification
            }
        }

        if(recieved_packet.header.type.type == ACKWM)
        {
            remote.unique_adrs = recieved_packet.header.magic_key;
            remote.ip_byte_adrs = recieved_packet.header.src;
        }
        inital_send = 0;
    }

    // DATA TRANSFER STATE
    // Initialize the PTU with data

    Data d;
    strcpy(d.data,data);
    d.type.type = DATA;
    Packet data_packet;
    data_packet.data = d;

    if((transmission_result = radio_send(remote.ip_byte_adrs,data_packet.raw,FRAME_PAYLOAD_SIZE)) < 0)
    {
        if(transmission_result == INVALID_ADRESS)
        {
            error.error_code = INVALID_ADRESS;
            strcpy(error.error_description,"The adress is not a valid IPv4 adress or port out of range");
        }
        else if (transmission_result == CONNECTION_ERROR) {
            error.error_code = INVALID_ADRESS;
            strcpy(error.error_description,"Have you paid your ISP bill or are you just trying to connect through your imagination?");
        }
    }
    Packet remote_response_packet;
    int response_code = 0;
    if((response_code = radio_recv(remote.unique_adrs,remote_response_packet.raw,to_ms)) < 0)
    {
        return response_code;
    }

    return transmission_result;
}

int ecg_recieve(int src, char *data, int _timeout)
{
    Packet _packet;

    memcpy(&_packet.raw,data,FRAME_PAYLOAD_SIZE);
}

void verifyChecksum()
{

}
