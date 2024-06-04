from serial import Serial
import socket
import time
import re
import os
import random

# MODIFY PORT AND BAUDRATE
#PORT = '/dev/cu.usbserial-0001' 
PORT = '/dev/cu.SLAB_USBtoUART' #--> UNCOMMENT THIS
# PORT = '/dev/cu.usbmodem131488301'
BAUDRATE = 38400
MAX_COUNT = -31 #######Change this

measurement = 'avionics'

field_keys = ["alt1", "alt2", "xacc", "yacc", "zacc", "lat", "lon", "siv"]

# just in case
def getTime():
    return time.time_ns()

start_time = str(getTime())
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# DO I NEED TO CHANGE SERVER ADDRESS PORT????
serverAddressPort = ('127.0.0.1', 20348)
ser = Serial(PORT, BAUDRATE, timeout=0.1)

# Modify value of N for varying smoothness
N = 3
packet_counter = 0


while True:
    timestamp = str(getTime())
    line2 = ser.readline() #--> UNCOMMENT THIS
    #packet_counter += 1

    if packet_counter != MAX_COUNT:
        #line = line2.decode() #--> UNCOMMENT THIS
        '''
        line = "104.69,490.11,0.90,8.62,4.64"
        random_lat_int = random.uniform(35.32, 35.33)
        line += f",{random_lat_int:.3f}"
        random_long_int = random.uniform(-117.77, -117.79)
        line += f",{random_long_int:.3f}"
        line += ",66"
        '''
        # Original string            
        print(line)
        # this starts the string at the first PT
        # Processing data
        data = line.split(',')
        #print(data)
        # Reset packet counter
        packet_counter = 0

        # combine all values together, separated by comma
        fields = ''
        for key, val in zip(field_keys, data):
            fields += f'{key}={val},'
        # delete comma after lon
        fields = fields.strip(',')

        # create influx string
        influx_string = measurement + ' ' + fields + ' ' + timestamp
        #print(influx_string)
        UDPClientSocket.sendto(influx_string.encode(), serverAddressPort)
        time.sleep(0.35)
