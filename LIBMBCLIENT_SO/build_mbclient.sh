#!/bin/bash
gcc -c -Wall -Werror -fpic mbclient.c
gcc -shared -o libmbclient.so mbclient.o libmodbus.a