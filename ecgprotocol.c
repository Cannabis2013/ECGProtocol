#include "ecgprotocol.h"


int ecg_send(int dst, char *data, int len)
{
    int bytes_send = 0;
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

        // Transmit the packet. Try to do so while obtaining results code indicating an error.

        int send_status = 0, try_connect = 4;
        while((send_status = radio_send(dst,_packet._raw,len) < 0))
        {
            if(send_status == INVALID_ADRESS)
            {
                // TODO: Implement some error notification

                return INVALID_ADRESS;
            }
            else if(send_status == CONNECTION_ERROR)
            {
                // TODO: Implement some error notification
                try_connect--;
            }
            if(try_connect < 0)
                return CONNECTION_ERROR;
        }

        // Clear the packet
        memset(_packet._raw,0,FRAME_PAYLOAD_SIZE);

        /* Await and recieve reply from remote. This is a blocking call
         * Timeout: 10 sec
         * 4 attempts for a total duration of 40 sec
         */

        int rcv_status = 0;
        int turn = 0;
        while((rcv_status =  radio_recv(&dst,_packet._raw,10000)) < 0 || ++turn > 4)
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
        inital_send = 0;
    }

    return bytes_send;
}

int ecg_recieve(int src, char *data, int _timeout)
{
    Frame _packet;

    memcpy(&_packet._raw,data,FRAME_PAYLOAD_SIZE);
}

void verifyChecksum()
{

}
