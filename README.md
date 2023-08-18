# ModBus Asheran Nuclear Simulator (MBANS)
MODBUS Asherah Nuclear Power Plant Executable with MODBUS server and ferryman. Linux Only at this time.

**Tested On:**

* Ubuntu 20.04.6 LTS (Focal Fossa)
* Python 3.8.10
* MATLAB R2020b (not required for this standalone executable version, just FYI)
 
## Requirements

### mbclient

mbclient is a shared library that allows a simple interface to implement Modbus client functionality in the simulator as well as other eventual applications. The shared library also allows independent applications and routines to share a single Modbus connection, in order to optimize resource usage.

The shared library **libmclient.so** is used by the client functions implemented in the Asherah simulator and is based on the open-source Modbus implementation [libmodbus](https://libmodbus.org/).

Note that the libmclient.so has to be in folder **/usr/lib** in order to be accessible to the client applications. Check that the user has permissions for this file too!

### Ferryman

Install the  `requirements.txt` using pip: `pip install -r requirements.txt`

I believe only the following are really needed: 

```
pymodbus==2.5.3
pyserial==3.5
requests==2.31.0
```

Note that updating past this version of `pymodbus` is going to bork the client used by `ferryman.py`
