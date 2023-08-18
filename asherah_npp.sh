 #!/bin/bash
 
#########################################################
##### Run this script with: $ sudo ./asherah_npp.sh #####
#########################################################

# Step 1: Start the modbus server via the modbus_server command in the 'MB_SERVER' folder:
echo "[MODBUS] Starting Modbus server..."
cd MB_SERVER/
sudo ./modbus_server &
echo "[MODBUS] Success."
cd ../

# Give it some time...
sleep 3s

####################################################
##### TWO OPTIONS FOR EXECUTING THE SIMULATION #####
####################################################
# Step 2: Start the simulator by running the applicable simulation executable in the 'SIMULATION' folder:
echo -e "\n[ANS_RT] Starting the Asherah Nuclear Simulator!"
cd  SIMULATION/

# Option 1: Nuclear Power Plant Simulator without Pressurizer control.
#           This allows the Pressurizer to be controlled by a PLC.
#           HIL = Hardware-in-the-Loop
#           RPS = Reactor Protection System
sudo ./alpha_mbans_HIL_RPS_ON &
cd ../

# Option 2: Nuclear Power Plant Simulator controlled entirely from internal controllers
            # This means no PLC or other exteneral controller(s) are required. RPS is ON.
            # sudo ./release_mbans_STANDALONE # Uncomment this line! Comment the other one on line 28!

##################################
##### OPTIONAL DATA EXCHANGE #####
##################################
# If you are running the `_HIL_RPS_ON` version, you need to control the model via MODBUS.
# ferryman.py in the 'FERRYMAN' folder is designed to do exactly that:
#                       1) Connect to the MB_SERVER started in this script and send the data to the PLC
#                       2) Connect to the PLC and send data to the MB_SERVER started in this script
# This essentially passes data between the two via MODBUS. Make sure to configure the IP addresses and data registers in the `ferryman.py` script.
#                       See also the `requirements.txt` file too!
cd FERRYMAN/
python3 ferryman.py
