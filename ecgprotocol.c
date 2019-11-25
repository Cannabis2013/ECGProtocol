#include "ecgprotocol.h"


void ecg_send(int dst, char *data, int len)
{
    Frame _packet;
    // TODO: Implement some initial functionality that signals the start of a transmission
    // NOTE: We may have to use threads in order to achieve this

    if(inital_send)
    {
        Header _initial_header;
        _initial_header._dst = (short) dst;
        _initial_header._src = (short) LOCALADRESS;
        _initial_header._type._type = INIT;

        _packet._header = _initial_header;

        int send_status = radio_send(dst,_packet._raw,len);

        if(send_status == INVALID_ADRESS)
        {
            // TODO: Implement some error notification
        }
        else if(send_status == CONNECTION_ERROR)
        {
            // TODO: Implement some error notification
        }

        // Clear the packet
        memset(_packet._raw,0,FRAME_PAYLOAD_SIZE);

        // Blocking call
        int rcv_status = radio_recv(&dst,_packet._raw,2000);

        if(rcv_status == INVALID_ADRESS)
        {
            // TODO: Implement some error notification
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

}

void ecg_recieve(int src, char *data, int _timeout)
{
    Frame _packet;

    memcpy(&_packet._raw,data,FRAME_PAYLOAD_SIZE);
}

void verifyChecksum()
{

}
