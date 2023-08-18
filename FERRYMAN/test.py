from pymodbus.client.sync import ModbusTcpClient
import requests
import time
from datetime import datetime

ans_srv_ip      = '10.16.0.92'#'192.168.10.34'
ans_srv_port    = 502
client          = None

input_reg = {
    "RC1_PumpDiffPress" : 5,
    "RC1_PumpSpeed" : 100,
    "RC1_PumpFlow" : 10000,
    "RC1_PumpTemp" : 1000,
    "RC2_PumpDiffPress" : 5,
    "RC2_PumpSpeed" : 100,
    "RC2_PumpFlow" : 10000,
    "RC2_PumpTemp" : 1000,
    "RX_MeanCoolTemp" : 1000,
    "RX_InCoolTemp" : 1000,
    "RX_OutCoolTemp" : 1000,
    "RX_CladTemp" : 1000,
    "RX_FuelTemp" : 1000,
    "RX_TotalReac" : 1.00E-05,
    "RX_ReactorPower" : 120,
    "RX_ReactorPress" : 20,
    "RX_CL1Press" : 20,
    "RX_CL2Press" : 20,
    "RX_CL1Flow" : 10000,
    "RX_CL2Flow" : 10000,
    "CR_Position" : 1000,
    "PZ_Press" : 20,
    "PZ_Temp" : 1000,
    "PZ_Level" :  10,
    "SG1_InletTemp" : 1000,
    "SG1_OutletTemp" : 1000,
    "SG2_InletTemp" : 1000,
    "SG2_OutletTemp" : 1000,
    "AF_MakeupValvePos" : 100,
    "AF_LetdownValvePos" : 100,
    "AF_MakeupFlow" : 1000,
    "AF_LetdownFlow" : 1000,
    "SG1_InletWaterTemp" : 1000,
    "SG1_OutletSteamTemp" : 1000,
    "SG1_InletWaterFlow" : 1000,
    "SG1_OutletSteamFlow" : 1000,
    "SG1_WaterTemp" : 1000,
    "SG1_SteamTemp" : 1000,
    "SG1_Press" : 20,
    "SG1_Level" : 20,
    "SG2_InletWaterTemp" : 1000,
    "SG2_OutletSteamTemp" : 1000,
    "SG2_InletWaterFlow" : 1000,
    "SG2_OutletSteamFlow" : 1000,
    "SG2_WaterTemp" : 1000,
    "SG2_SteamTemp" : 1000,
    "SG2_Press" : 20,
    "SG2_Level" : 20,
    "SD_CtrlValvePos" : 100,
    "TB_Speed" : 250,
    "TB_InSteamPress" : 20,
    "TB_OutSteamPress" : 10000,
    "TB_SpeedCtrlValvePos" : 100,
    "TB_InSteamFlow" : 2000,
    "GN_GenElecPow" : 1000,
    "GN_GridFreq" : 100,
    "GN_GenFreq" : 100,
    "CD_Level" : 2,
    "CD_SteamTemp" : 1000,
    "CD_CondTemp" : 1000,
    "CD_Press" : 10000,
    "CD_InSteamFlow" : 2000,
    "CD_OutCondFlow" : 2000,
    "CC_PumpInletTemp" : 1000,
    "CC_PumpOutletTemp" : 1000,
    "CC_PumpSpeed" : 100,
    "CC_PumpFlow" : 2.00E+05,
    "CC_PumpTemp" : 1000,
    "FW_TankPress" : 2,
    "FW_TankLevel" : 10,
    "FW_Pump1DiffPress" : 20,
    "FW_Pump1Flow" : 1000,
    "FW_Pump1Speed" : 120,
    "FW_Pump1Temp" : 1000,
    "FW_Pump2DiffPress" : 20,
    "FW_Pump2Flow" : 1000,
    "FW_Pump2Speed" : 120,
    "FW_Pump2Temp" : 1000,
    "FW_Pump3DiffPress" : 20,
    "FW_Pump3Flow" : 1000,
    "FW_Pump3Speed" : 120,
    "FW_Pump3Temp" : 1000,
    "CE_Pump1DiffPress" : 20,
    "CE_Pump1Speed" : 120,
    "CE_Pump1Flow" : 1000,
    "CE_Pump1Temp" : 1000,
    "CE_Pump2DiffPress" : 20,
    "CE_Pump2Speed" : 120,
    "CE_Pump2Flow" : 1000,
    "CE_Pump2Temp" : 1000,
    "CE_Pump3DiffPress" : 20,
    "CE_Pump3Speed" : 120,
    "CE_Pump3Flow" : 1000,
    "CE_Pump3Temp" : 1000,
    "INT_SimulationTime" : 65535
}

# unused
hold_reg = {
    "RC1_PumpSpeedCmd": 100,
    "RC2_PumpSpeedCmd": 100,
    "CR_PosCmd": 1000,
    "PZ_MainHeaterPowCmd": 100,
    "PZ_CL1SprayValveCmd": 100,
    "PZ_CL2SprayValveCmd": 100,
    "AF_MakeupValveCmd": 100,
    "AF_LetdownValveCmd": 100,
    "SD_CtrlValveCmd": 100,
    "TB_SpeedCtrlValveCmd": 100,
    "CC_PumpSpeedCmd": 100,
    "FW_Pump1SpeedCmd": 100,
    "FW_Pump2SpeedCmd": 100,
    "FW_Pump3SpeedCmd": 100,
    "CE_Pump1SpeedCmd": 100,
    "CE_Pump2SpeedCmd": 100,
    "CE_Pump3SpeedCmd": 100,
    "CTRL_RXPowerSetpoint": 110,
    "CTRL_PZPressSetPoint": 18000000
}

max_range = [100,100,1000,100,100,100,100,100,100,100,100,100,100,100,100,100,100,110,18000000]

def read_input_register():
    #val = client.read_inpu
    global client
    dt = datetime.now()
    ts = datetime.timestamp(dt)
    print("TIME:", ts)
    regii = client.read_input_registers(0, 94)
    list_input_regs = list(input_reg.items())
    i = 0
    
    for reg in regii.registers:
        current_reg = list_input_regs[i]
        reg_value = reg / 65535 * current_reg[1]
        print(f'\t[>] {current_reg[0]} : {reg_value}')
        i = i + 1
    time.sleep(1)
    clear_console(95)
    # for key, value in input_reg.items():
    #     print(f'\t[>] {key}: {value}')

def read_holding_registers():
    # TODO: Code Here, Input Register 00021
    global client
    global max_range
    regii = client.read_holding_registers(0, 19)
    #print(type(regii.registers))
    time.sleep(1)
    i = 0 
    dt = datetime.now()
    ts = datetime.timestamp(dt)
    print("TIME:", ts)
    for reg in regii.registers:
        reg = reg / 65535 * max_range[i]
        print(f'\t[>] Register {i}: {reg}')
        i = i + 1
        

def RXPowerSetpoint_test(new_power_setpoint):
    # TODO: Code Here, Holding Register 00017
    global client
    # RESETS TO NORMAL SETPOINT IN CASE OF RERUNNING THE SCRIPT
    resp = client.write_register(17, 54613, unit=1)
    # Scale the power setpoint
    new_power_setpoint = int(new_power_setpoint / 120 * 65535)
        
    for i in range(100):
        # Read the power and scale the value
        reg = client.read_input_registers(14, count=1, unit=1)
        rx_pwr = reg.registers[0] / 65535 * 120
        
        reg = client.read_holding_registers(17, 1)
        rx_pwr_setpoint = reg.registers[0] / 65535 * 120
        
        if i == 12:
            resp = client.write_register(17, new_power_setpoint, unit=1)
        dt = datetime.now()
        ts = datetime.timestamp(dt)    
        print(f'\t[{i}>] RX_PWR Setpoint    : {rx_pwr_setpoint}')
        print(f'\t[ >] RX_PWR Value       : {rx_pwr}')
        
        time.sleep(1)
        clear_console(2)
    

def PRZPowerSetpoint_test(new_power_setpoint):
    # TODO: Code Here, Holding Register 00018
    global client
    # RESETS TO NORMAL SETPOINT IN CASE OF RERUNNING THE SCRIPT
    resp = client.write_register(18, 54998, unit=1)
    # Scale the power setpoint
    new_power_setpoint = int(new_power_setpoint / 18000000 * 65535)
        
    for i in range(100):
        # Read the pressure and scale the value
        reg = client.read_input_registers(21, count=1, unit=1)
        pzr_pwr = reg.registers[0] / 65535 * 20000000
        
        reg = client.read_holding_registers(18, 1)
        pzr_pwr_setpoint = reg.registers[0] / 65535 * 18000000
        
        if i == 12:
            resp = client.write_register(18, new_power_setpoint, unit=1)
        dt = datetime.now()
        ts = datetime.timestamp(dt)    
        print(f'\t[{i}>] PZR_PWR Setpoint    : {pzr_pwr_setpoint}')
        print(f'\t[ >] PZR_Press Value       : {pzr_pwr}')
        
        time.sleep(1)
        clear_console(2)

def clear_console(num_lines):
    LINE_UP = '\033[1A'
    LINE_CLEAR = '\x1b[2K'
    for i in range(num_lines):
        print(LINE_UP, end=LINE_CLEAR)
   

def test():
    global client
    reg = client.read_holding_registers(1024, 7)
    print(reg.registers)
    reg = client.write_register(1024, 49478, unit=1)
    time.sleep(1)
    reg = client.read_holding_registers(1024, 7)
    print(reg.registers)

def connect_to_server():
    print(f'[INFO] Attempting to connect to server on tcp://{ans_srv_ip}:{ans_srv_port}...')
    global client 
    client = ModbusTcpClient(ans_srv_ip)
    client.connect()

        
if __name__ == "__main__":
    try:
        connect_to_server()
    except Exception as e: 
        print(f"[FAIL] Cannot connect to server. More info: {e}")
        exit()
        
    test()
    # while True:
    #     #write_CTRL_RXPowerSetpoint()
        
    #     # read_holding_registers()
    #     # clear_console(20)
        
    #     read_input_register()
    
        
    #RXPowerSetpoint_test(50)
    # PRZPowerSetpoint_test(15100000)