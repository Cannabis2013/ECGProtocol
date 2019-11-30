#include <stdio.h>
#include "radio.h"
#include <string.h>

#include "ecgprotocol.h"

int main()
{
    char *data = "Hej med dig";
    ecg_init(42000);
    ecg_send(45000,"Hej med dig",11,5000);
    return 0;
}
