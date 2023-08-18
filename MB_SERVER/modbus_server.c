/* MODBUS_SERVER.C
 * Adapted from the work below
 * RP Marques May/22
 * /

/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <modbus/modbus.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MB_MAXCONNECT 64

#define MB_MAXADDR 1024

#define MB_PORT_FILE "modbus_srv_port.txt"

#define INIT_DISCINP "modbus_init_discinp.txt"
#define INIT_COIL    "modbus_init_coil.txt"
#define INIT_INPREG  "modbus_init_inpreg.txt"
#define INIT_HOLDREG "modbus_init_holdreg.txt"
#define INIT_LINELEN  128

#define DISCINP_FILE "/ramdisk/discrete_inputs.txt"
#define INPREG_FILE  "/ramdisk/input_registers.txt"
#define MAXTRY 10

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;

static int server_socket = -1;

static void close_sigint(int dummy)
{
    if (server_socket != -1)
    {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

int main(void)
{
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    int fdmax;
    FILE *f;
    FILE *finit;
    FILE *fcurr;
    char fileline[INIT_LINELEN];
    int i;
    int mb_port;
    int addr_buf;
    int coil_buf;
    int ninpcoil;
    int ninpreg;
    int addr[MB_MAXADDR];
    int coil[MB_MAXADDR];
    int reg[MB_MAXADDR];
    int reg_buf;
    float val_buf, min_buf, max_buf;
    char tag[INIT_LINELEN];
    int try;
    int fd;

    /* Get port from file */
    f = fopen(MB_PORT_FILE, "r");
    if (f == NULL)
    {
        printf("## MB_SRV:  Fail opening file %s.\n", MB_PORT_FILE);
        return -1;
    }
    fscanf(f, "%d", &mb_port);
    fclose(f);

    /* Advertise */
    printf("## MB_SRV:  Local Modbus server.\n");
    printf("## MB_SRV:  Listening in all IP addresses at port %d.\n", mb_port);

    ctx = modbus_new_tcp(NULL, mb_port);
    if (ctx == NULL)
    {
        printf("## MB_SRV: Unable to allocate libmodbus context.\n");
        return -1;
    }

    mb_mapping = modbus_mapping_new(MB_MAXADDR, MB_MAXADDR, MB_MAXADDR, MB_MAXADDR);
    if (mb_mapping == NULL)
    {
        printf("## MB_SRV: Failed to allocate the mapping: %s.\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Initialize mapping from files */
    /* Discrete inputs */
    finit = fopen(INIT_DISCINP, "r");
    if (finit == NULL)
    {
        printf("## MB_SRV:  Failed to open init file %s.\n", INIT_DISCINP);
        return -1;
    }
    i = 0;
    while (fgets(fileline, MB_MAXADDR, finit))
    {
        /* adjust string */
        if (fileline[strlen(fileline) - 1] == '\n')
        {
            fileline[strlen(fileline) - 1] = '\0';            
        }
        if (fileline[strlen(fileline) - 1] == '\r')
        {
            fileline[strlen(fileline) - 1] = '\0';
        }
        sscanf(fileline, "%d %s %d", &addr[i], tag, &coil[i]);
        mb_mapping->tab_input_bits[addr[i]] = (uint8_t) coil[i];
        i++;
    }
    ninpcoil = i;
    fclose(finit);
    /* create first exchange data file with initial conditions for discrete inputs */
    fcurr = fopen(DISCINP_FILE, "w+");
    if (fcurr == NULL)
    {
        printf("## MB_SRV: Failed to create file %s.\n", DISCINP_FILE);
        return -1;
    }
    chmod(DISCINP_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    fprintf(fcurr, "%d %d ",addr[0], coil[0]);
    //printf("%d %d\n",addr[0],coil[0]);
    for (i=1; i < ninpcoil; i++)
    {
        fprintf(fcurr, "\n%d %d ",addr[i], coil[i]);
        //printf("%d %d\n",addr[i],coil[i]);
    }
    printf("\n");
    fclose(fcurr);
    /* Coils */
    finit = fopen(INIT_COIL, "r");
    if (finit == NULL)
    {
        printf("## MB_SRV: Failed to open init file %s.\n", INIT_COIL);
        return -1;
    }
    while (fgets(fileline, MB_MAXADDR, finit))
    {
        /* adjust string */
        if (fileline[strlen(fileline) - 1] == '\n')
        {
            fileline[strlen(fileline) - 1] = '\0';            
        }
        if (fileline[strlen(fileline) - 1] == '\r')
        {
            fileline[strlen(fileline) - 1] = '\0';
        }
        sscanf(fileline, "%d %s %d", &addr_buf, tag, &coil_buf);
        mb_mapping->tab_bits[addr_buf] = coil_buf;
    } 
    fclose(finit);   
    /* Input Registers */
    finit = fopen(INIT_INPREG, "r");
    if (finit == NULL)
    {
        printf("## MB_SRV: Failed to open init file %s.\n", INIT_INPREG);
        return -1;
    }
    i = 0;
    while (fgets(fileline, MB_MAXADDR, finit))
    {
        // adjust string
        if (fileline[strlen(fileline) - 1] == '\n')
        {
            fileline[strlen(fileline) - 1] = '\0';            
        }
        if (fileline[strlen(fileline) - 1] == '\r')
        {
            fileline[strlen(fileline) - 1] = '\0';
        }
        sscanf(fileline, "%d %s %g %g %g", &addr[i], tag, &val_buf, &min_buf, &max_buf);
        if (val_buf > max_buf) reg[i] = 65535;
        else if (val_buf < min_buf) reg[i] = 0;
        else reg[i] = 65535.0*(val_buf - min_buf)/(max_buf - min_buf);
        mb_mapping->tab_input_registers[addr[i]] = (uint16_t) reg[i];
        i++;
    }
    ninpreg = i;
    fclose(finit);
    /* create first exchange data file with init values of input registers */
    fcurr = fopen(INPREG_FILE, "w+");
    if (fcurr == NULL)
    {
        printf("## MB_SRV: Failed to create file %s.\n", INPREG_FILE);
        return -1;
    }
    chmod(INPREG_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    fprintf(fcurr, "%d %d ",addr[0], reg[0]);
    for (i=1; i < ninpreg; i++)
    {
        fprintf(fcurr, "\n%d %d ",addr[i], reg[i]);
    }
    fclose(fcurr);
    /* Holding Registers */
    finit = fopen(INIT_HOLDREG, "r");
    if (finit == NULL)
    {
        printf("## MB_SRV: Failed to open init file %s.\n", INIT_HOLDREG);
        return -1;
    }
    while (fgets(fileline, MB_MAXADDR, finit))
    {
        // adjust string
        if (fileline[strlen(fileline) - 1] == '\n')
        {
            fileline[strlen(fileline) - 1] = '\0';            
        }
        if (fileline[strlen(fileline) - 1] == '\r')
        {
            fileline[strlen(fileline) - 1] = '\0';
        }
        sscanf(fileline, "%d %s %g %g %g", &addr_buf, tag, &val_buf, &min_buf, &max_buf);
        if (val_buf > max_buf) reg_buf = 65535;
        else if (val_buf < min_buf) reg_buf = 0;
        else reg_buf = (uint16_t) 65535.0*(val_buf - min_buf)/(max_buf - min_buf);
        mb_mapping->tab_registers[addr_buf] = (uint16_t) reg_buf;
    } 
    fclose(finit);

    server_socket = modbus_tcp_listen(ctx, MB_MAXCONNECT);
    if (server_socket == -1)
    {
        printf("## MB_SRV: Unable to listen to TCP connection.\n");
        modbus_free(ctx);
        return -1;
    }

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for (;;)
    {
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1)
        {
            perror("## MB_SRV: Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++)
        {

            if (!FD_ISSET(master_socket, &rdset))
            {
                continue;
            }

            if (master_socket == server_socket)
            {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
                if (newfd == -1)
                {
                    perror("## MB_SRV: Server accept() error");
                }
                else
                {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax)
                    {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("## MB_SRV: New connection from %s:%d on socket %d.\n",
                           inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            }
            else
            {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0)
                {
                    /* Read discrete inputs from file */
                    fcurr = NULL; try = 0;
                    while ((fcurr == NULL) && (try < MAXTRY))
                    {
                        if (try >0)
                        {
                            usleep(900);
                        }
                        fcurr = fopen(DISCINP_FILE, "r");
                    }
                    if (fcurr == NULL)
                    {
                        printf("## MB_SRV: Failed to open discrete input file %s.\n", DISCINP_FILE);
                        return -1;
                    }
                    /* Lock file during operation */
                    fd = fileno(fcurr);
                    flock(fd, LOCK_EX);
                    while (fgets(fileline, MB_MAXADDR, fcurr))
                    {
                        /* adjust string */
                        if (fileline[strlen(fileline) - 1] == '\n')
                        {
                            fileline[strlen(fileline) - 1] = '\0';
                        }
                        if (fileline[strlen(fileline) - 1] == '\r')
                        {
                            fileline[strlen(fileline) - 1] = '\0';
                        }
                        sscanf(fileline, "%d %d", &addr_buf, &coil_buf);
                        mb_mapping->tab_input_bits[addr_buf] = (uint8_t) coil_buf;
                    }
                    /* Release lock and close file */
                    flock(fd, LOCK_UN);
                    fclose(fcurr);
                    /* Read input registers from file */
                    fcurr = NULL; try = 0;
                    while ((fcurr == NULL) && (try < MAXTRY))
                    {
                        if (try >0)
                        {
                            usleep(900);
                        }
                        fcurr = fopen(INPREG_FILE, "r");
                    }
                    if (fcurr == NULL)
                    {
                        printf("## MB_SRV: Failed to open input register file %s.\n", INPREG_FILE);
                        return -1;
                    }
                    /* Lock file during operation */
                    fd = fileno(fcurr);
                    flock(fd, LOCK_EX);
                    while (fgets(fileline, MB_MAXADDR, fcurr))
                    {
                        /* adjust string */
                        if (fileline[strlen(fileline) - 1] == '\n')
                        {
                            fileline[strlen(fileline) - 1] = '\0';
                        }
                        if (fileline[strlen(fileline) - 1] == '\r')
                        {
                            fileline[strlen(fileline) - 1] = '\0';
                        }
                        sscanf(fileline, "%d %d", &addr_buf, &reg_buf);
                        mb_mapping->tab_input_registers[addr_buf] = (uint16_t) reg_buf;
                    }
                    /* Release lock and close file */
                    flock(fd, LOCK_UN);
                    fclose(fcurr);
                    modbus_reply(ctx, query, rc, mb_mapping);
                }
                else if (rc == -1)
                {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("## MB_SRV: Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax)
                    {
                        fdmax--;
                    }
                }
            }
        }
    }

    return 0;
}