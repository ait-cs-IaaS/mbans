from pymodbus.client.sync import ModbusTcpClient
import requests
import time
from datetime import datetime

###########################################################

# TODO: Cast all values to UNINT 16

###########################################################

# mbans connection
mbans_srv_ip      = '10.16.0.34' # TODO: Replace IP
mbans_srv_port    = 502
mbans_client      = None

# plc connection
plc_srv_ip      = '10.16.0.92' # TODO: Replace ip
plc_srv_port    = 502
plc_client      = None

# Variables to Exchange
pzr_press               = None
ctrl_pzr_press_setpoint = None
pzr_main_heater         = None
pzr_backup_heater       = None
pzr_cl1_spray           = None
pzr_cl2_spray           = None

###########################################################

# Take data from Asherah, and send to the PLC
def read_sensor_data():
    global pzr_press
    # Read Pressure from Asherah (MBANS)
    r_pzr_resp      = mbans_client.read_input_registers(21,1)
    pzr_press       = r_pzr_resp.registers[0]
    scaled_press    = pzr_press / 65535 * 20000000
    print(f'[SENSOR TRANSMIT] Reading raw value from Asherah : UINT {pzr_press} || {scaled_press}MPa.')
    
    # Write value to the PLC
    w_prz_resp  = plc_client.write_register(1024, pzr_press, unit=1)
    print(f'[SENSOR TRANSMIT] Writing value to the PLC : UINT {pzr_press}.')

# Take the control actions from the PLC and write them to Asherah
def write_control_commands():
    global pzr_main_heater
    global pzr_backup_heater
    global pzr_cl1_spray
    global pzr_cl2_spray
    
    # Read holding registers of PLC
    holding_reg  = plc_client.read_holding_registers(1026,3) # READ 3 CONSECUTIVE HOLDING REGISTERS IN THE PLC
    pzr_main_heater     = holding_reg.registers[0]
    scaled_main_heater  = pzr_main_heater / 65535 * 100
    print(f'[CONTROL COMMAND] Reading control from PLC : UINT {pzr_main_heater} || {scaled_main_heater}%.')
    pzr_cl1_spray       = holding_reg.registers[1]
    scaled_cl1_spray    = pzr_cl1_spray / 32767 * 100
    print(f'[CONTROL COMMAND] Reading control from PLC : UINT {pzr_cl1_spray} || {scaled_cl1_spray}%.')
    pzr_cl2_spray       = holding_reg.registers[2]
    scaled_cl2_spray    = pzr_cl2_spray / 32767 * 100
    print(f'[CONTROL COMMAND] Reading control from PLC : UINT {pzr_cl2_spray} || {scaled_cl2_spray}%.')
    
    # Read coils (binary outputs) from PLC
    coil_reg = plc_client.read_coils(0, 1)
    pzr_backup_heater = coil_reg.bits[0]
    print(f'[CONTROL COMMAND] Reading control from PLC : BOOL {pzr_backup_heater}.')
    
    # Write command to Asherah (MBANS)
    w_prz_main_heater_resp = mbans_client.write_register(3, pzr_main_heater, unit=1)
    print(f'[CONTROL COMMAND] Writing value to Asherah : UINT {pzr_main_heater} : Main Heater')
    w_prz_cl1_spray_resp = mbans_client.write_register(4, pzr_cl1_spray, unit=1)
    print(f'[CONTROL COMMAND] Writing value to Asherah : UINT {pzr_cl1_spray} : Spray Valve 1.')
    w_prz_cl2_spray_resp = mbans_client.write_register(5, pzr_cl2_spray, unit=1)
    print(f'[CONTROL COMMAND] Writing value to Asherah : UINT {pzr_cl2_spray} : Spray Valve 2')
    w_prz_backup_heater_resp = mbans_client.write_coil(3, pzr_backup_heater)
    print(f'[CONTROL COMMAND] Writing value to Asherah : BOOL {pzr_backup_heater} : Backup Heater')

# Connect to the PLC and Asherah
def connect_to_servers():
    print(f'[INFO] Attempting to connect to ANS on tcp://{mbans_srv_ip}:{mbans_srv_port}...')
    global mbans_client
    global plc_client
    
    # mbans Connection
    mbans_client = ModbusTcpClient(mbans_srv_ip)
    mbans_client.connect()
    
    # PLC Connection
    plc_client = ModbusTcpClient(plc_srv_ip)
    plc_client.connect()

def clear_console(num_lines):
    LINE_UP = '\033[1A'
    LINE_CLEAR = '\x1b[2K'
    for i in range(num_lines):
        print(LINE_UP, end=LINE_CLEAR)

###########################################################
        
if __name__ == "__main__":
    try:
        connect_to_servers()
    except Exception as e: 
        print(f"[FAIL] Cannot connect to one or more servers. More info: {e}")
        exit()
    
    while True:
        dt = datetime.now()
        ts = datetime.timestamp(dt)
        print("TIME:", ts)
        read_sensor_data()
        write_control_commands()
        time.sleep(1)
        clear_console(11)
