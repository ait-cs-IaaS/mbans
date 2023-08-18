# ModBus Asheran Nuclear Simulator (MBANS)
MODBUS Asherah Nuclear Power Plant Executable with MODBUS server and ferryman. Linux Only at this time.

**Tested On:**

* Ubuntu 20.04.6 LTS (Focal Fossa)
* Python 3.8.10
* MATLAB R2020b (not required for this standalone executable version, just FYI)

## Acknowledgement

The Asherah Nuclear (Power Plant) Simulator (a.k.a. ANS) is a MATLAB Simulink model of a Twin-Loop Pressurized Water Reactor (PWR) Nuclear Power Plant (NPP). It was developed as part of the International Atomic Energy Agency (IAEA) [Coordinated Research Project (CRP) J02008](https://www.iaea.org/projects/crp/j02008). 

The University of São Paulo, Brazil, are responsible for the model's development. Special thanks for the MODBUS/TCP version of the model used in this repository must go to Professor Ricardo Paulino Marques, who worked on this model at the request of AIT Austrian Institute of Technology GmbH. Modifications made by Prof. Marques mean that software and hardware-based controllers can be used to control the simulator via the MODBUS/TCP protocol.
 
## Requirements

### mbclient

mbclient is a shared library that allows a simple interface to implement Modbus client functionality in the simulator as well as other eventual applications. The shared library also allows independent applications and routines to share a single Modbus connection, in order to optimize resource usage.

The shared library **libmclient.so** is used by the client functions implemented in the Asherah simulator and is based on the open-source Modbus implementation [libmodbus](https://libmodbus.org/).

Note that the libmclient.so has to be in folder **/usr/lib** in order to be accessible to the client applications. Check that the user has permissions for this file too!

### Ferryman

Ferryman is a python script that connects to two MODBUS endpoints to exchange data via MODBUS/TCP. The version in this repository connects to an OpenPLC server and the MODBUS server in `MB_SERVER` to exchange Pressurizer process control signals.

Install the  `requirements.txt` using pip: `pip install -r requirements.txt`

I believe only the following are really needed: 

```
pymodbus==2.5.3
pyserial==3.5
requests==2.31.0
```

Note that updating past this version of `pymodbus` is going to bork the client used by `ferryman.py`

## Directory Structure

The following directory structure is defined:

'''
.
├── DEPENDENCIES
│   └── libmbclient.so <-- see Requirements->mbclient
├── FERRYMAN
│   ├── ferryman.py <-- see Requirements->Ferryman
│   ├── requirements.txt
│   └── test.py <-- ignore for the most part
├── MB_SERVER
│   ├── libmodbus.a
│   ├── modbus
│   │   ├── modbus-rtu.h
│   │   ├── modbus-tcp.h
│   │   ├── modbus-version.h
│   │   └── modbus.h
│   ├── modbus_init_coil.txt
│   ├── modbus_init_discinp.txt
│   ├── modbus_init_holdreg.txt
│   ├── modbus_init_inpreg.txt
│   ├── modbus_server <-- starts the local MODBUS server for the model
│   ├── modbus_server.c
│   └── modbus_srv_port.txt
├── SIMULATION
│   ├── alpha_mbans_HIL_RPS_ON <-- NPP simulation for external control (requires ferryman.py)
│   └── release_mbans_STANDALONE <-- NPP standalone simulation (does not require ferryman.py)
└── asherah_npp.sh <-- main script for running the different services
''' 
