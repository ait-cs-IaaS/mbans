
/* Defines */
#define MB_IP_FILE "modbus_srv_ip.txt"
#define MB_PORT_FILE "modbus_srv_port.txt"
#define NTRYMAX 30

/* Includes */
#include "modbus.h"
#include <stdio.h>
#include <unistd.h>

/* Shared variables */
static int mb_connected = 0;
static modbus_t* mb;
static int mb_port;
static char mb_ip_address[32];

/* Functions */
int mbclient_connect(void)
{
    if (mb_connected == 0)
    {
        FILE *f;
        int ntry = 0;

        /* Get IP Address from file */
        f = fopen(MB_IP_FILE, "r");
        if (f == NULL)
        {
            printf("## mb_client_connect:  Fail opening file %s.\n", MB_IP_FILE);
            return 0;
        }
        fscanf(f, "%s", mb_ip_address);
        fclose(f);

        /* Get port from file */
        f = fopen(MB_PORT_FILE, "r");
        if (f == NULL)
        {
            printf("## mb_client_connect:  Fail opening file %s.\n", MB_PORT_FILE);
            return 0;
        }
        fscanf(f, "%d", &mb_port);
        fclose(f);

        /* Create connection to server */
        while (ntry < NTRYMAX)
        {
            mb = modbus_new_tcp(mb_ip_address, mb_port);
            if (modbus_connect(mb) == -1)
            {
                sleep(1);
                ntry = ntry+1;
            }
            else
            {
                mb_connected = 1;
                break;
            }
        }
        if (mb_connected == 0)
        {
            printf("## mb_client_connect: Fail connecting to server %s:%d.\n",mb_ip_address,mb_port);
            modbus_free(mb);
            return 0;
        }
    }
    else
    {
        return 1;
    }
    printf("## mb_client_connect: Connected to server %s:%d.\n",mb_ip_address,mb_port);
    return 1;
}

int mbclient_disconnect(void)
{
    if (mb_connected == 1)
    {
        modbus_free(mb);
        mb_connected = 0;
        printf("## mb_client_disconnect: Disconnected from server %s:%d.\n",mb_ip_address,mb_port);
    }
    else
    {
        printf("## mb_client_disconnect: Client was not connected to server %s:%d.\n",mb_ip_address,mb_port);       
    }
    return 0;
}

int mbclient_read_coil(int addr, int nb, uint8_t* dest)
{
    int res;
    if (mb_connected == 1)
    {
        res = modbus_read_bits(mb, addr, nb, dest);
        if (res == -1)
        {
            printf("## mb_client_read_coil: Read error.\n");
        }
        return res;
    }
    printf("## mb_client_read_coil: not connected to server.\n");
    return -1;
}

int mbclient_read_holdreg(int addr, int nb, uint16_t* dest)
{
    int res;
    if (mb_connected == 1)
    {
        res = modbus_read_registers(mb, addr, nb, dest);
        if (res == -1)
        {
            printf("## mb_client_read_holdreg: Read error.\n");
        }
        return res;
    }
    printf("## mb_client_read_holdreg: not connected to server.\n");
    return -1;    
}

int mbclient_read_discinp(int addr, int nb, uint8_t* dest)
{
    int res;
    if (mb_connected == 1)
    {
        res = modbus_read_input_bits(mb, addr, nb, dest);
        if (res == -1)
        {
            printf("## mb_client_read_discinp: Read error.\n");
        }
        return res;
    }
    printf("## mb_client_read_discinp: not connected to server.\n");
    return -1;
}

int mbclient_read_inpreg(int addr, int nb, uint16_t* dest)
{
    int res;
    if (mb_connected == 1)
    {
        res = modbus_read_input_registers(mb, addr, nb, dest);
        if (res == -1)
        {
            printf("## mb_client_read_inpreg: Read error.\n");
        }
        return res;
    }
    printf("## mb_client_read_inpreg: not connected to server.\n");
    return -1;    
}

int mbclient_write_coil(int addr, int nb, const uint8_t* src)
{
    int res;
    if (mb_connected == 1)
    {
        res = modbus_write_bits(mb, addr, nb, src);
        if (res == -1)
        {
            printf("## mb_client_write_coil: Write error.\n");
        }
        return res;
    }
    printf("## mb_client_write_coil: not connected to server.\n");
    return -1;
}

int mbclient_write_holdreg(int addr, int nb, const uint16_t* src)
{
    int res;
    if (mb_connected == 1)
    {
        res = modbus_write_registers(mb, addr, nb, src);
        if (res == -1)
        {
            printf("## mb_client_write_holdreg: Write error.\n");
        }
        return res;
    }
    printf("## mb_client_write_holdreg: not connected to server.\n");
    return -1;
}
