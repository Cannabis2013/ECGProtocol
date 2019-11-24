#include "ecgprotocol.h"


void ecg_send(int dst, char *data, int len)
{
    Frame _packet;
    // TODO: Implement some initial functionality that signals the start of a transmission
    // NOTE: We may have to use threads in order to achieve this

    if(inital_send)
    {

    }

}

void ecg_recieve(int src, char *data, int _timeout)
{
    Frame _packet;
}

void verifyChecksum()
{

}
