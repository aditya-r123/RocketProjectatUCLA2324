#!/bin/bash

# Define the directory
directory="/Users/adityar/Downloads/RocketProjectatUCLA2324/Ground_Systems/Grafana_Files/"


# Navigate to the directory
cd "$directory" || { echo "Directory not found"; exit 1; }

# Check available terminal emulators (replace with appropriate one)
terminal_emulator="xterm"

# Open a new terminal window and execute the first Python file
$terminal_emulator -e "telegraf --config SensorVals.conf" &

# Open a new terminal window and execute the second Python file
$terminal_emulator -e "telegraf --config OctocouplerStates.conf" &

# Execute the third Python file in the current terminal window
python3 Grafana_Final.py
