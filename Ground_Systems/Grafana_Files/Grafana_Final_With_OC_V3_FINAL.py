from serial import Serial #--> UNCOMMENT THIS
import socket
import time
import re
import os
import random

PORT = '/dev/cu.SLAB_USBtoUART' #--> UNCOMMENT THIS
BAUDRATE = 115200
MAX_COUNT = -31 #######Change this

measurement = 'sensorvals'
measurement2 = 'octovals'

field_keys = ["pt1", "pt2", "pt3", "pt4", "pt5", "pt6", "lc1", "lc2"]
field_keys2 = ["ignite", "fill", "dump", "vent", "qd", "purge", "mpv", "siren"]

# just in case
def getTime():
    return time.time_ns()

start_time = str(getTime())
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

serverAddressPort = ('127.0.0.1', 4001)
serverAddressPort2 = ('127.0.0.1', 65415)
ser = Serial(PORT, BAUDRATE, timeout=0.1) #--> UNCOMMENT THIS

# Modify value of N for varying smoothness
N = 3
packet_counter = 0

while True:
    try:
        print('ser\n')
        print(ser)
        ser = Serial(PORT, BAUDRATE, timeout=0.1)
        print('ser\n')
        print("ser")
        print('ser\n')
    except:
        continue
    finally:
        print('Successfully connected to Serial device')
        break

while True:
    timestamp = str(getTime())
    line2 = ser.readline() #--> UNCOMMENT THIS
    #packet_counter += 1
    if packet_counter != MAX_COUNT:
        line = line2.decode() #--> UNCOMMENT THIS
        #line = "10,20,30,40,90,60,100,300,101011011" #--> COMMENT THIS
        line = line.strip().replace(" ", "")  # Clean the input line
        # Split the input line into individual values
        data = line.split(',')
        opto_dataa = ", ".join(data[-1])
        print(opto_dataa)
        opto_data = [int(x.strip()) for x in opto_dataa.split(',')]
        sensor_data = data[:-1]
        print(sensor_data)
        packet_counter = 0
        fields = ''
        for key, val in zip(field_keys, sensor_data):
            fields += f'{key}={val},'
        fields = fields.strip(',')
        influx_string = measurement + ' ' + fields + ' ' + timestamp
        print(influx_string)
        UDPClientSocket.sendto(influx_string.encode(), serverAddressPort)
        fields2 = ''
        for key2, val2 in zip(field_keys2, opto_data):
            fields2 += f'{key2}={val2},'
        fields2 = fields2.strip(',')
        influx_string2 = measurement2 + ' ' + fields2
        print(influx_string2)
        UDPClientSocket.sendto(influx_string2.encode(), serverAddressPort2)
