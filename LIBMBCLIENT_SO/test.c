#include <stdio.h>
#include "mbclient.h"
#include <unistd.h>

int main(void)
{
    int mb_connected;
    modbus_t* mb;

    mb = mbclient_connect();
    if ( mb == NULL)
    {
        mb_connected = 0;
    }
    else
    {
        mb_connected = 1;
    }
    
    if (mb_connected == 0)
    {
        printf(">> Failure connecting\n");
    }
    else
    {
        printf(">> Success.\n");
    }
    sleep(5);
    mb_connected = mbclient_disconnect();


    

}