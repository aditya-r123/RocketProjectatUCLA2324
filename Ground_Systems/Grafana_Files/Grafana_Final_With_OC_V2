#from serial import Serial
import socket
import time
import re
import os
import random

# MODIFY PORT AND BAUDRATE
# PORT = '/dev/cu.usbserial-0001' 
#PORT = '/dev/cu.SLAB_USBtoUART' #--> UNCOMMENT THIS
# PORT = '/dev/cu.usbmodem131488301'
BAUDRATE = 115200
MAX_COUNT = -31 #######Change this

measurement = 'sensorvals'
measurement2 = 'octovals'

field_keys = ["pt1", "pt2", "pt3", "pt4", "pt5", "pt6", "lc1", "lc2"]
field_keys2 = ["siren", "ignite", "fill", "vent", "dump", "qd", "mpv", "purge"]

# just in case
def getTime():
    return time.time_ns()

start_time = str(getTime())
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

serverAddressPort = ('127.0.0.1', 4001)
serverAddressPort2 = ('127.0.0.1', 65415)
#ser = Serial(PORT, BAUDRATE, timeout=0.1)

# Modify value of N for varying smoothness
N = 3
packet_counter = 0

while True:
    try:
        #print('ser\n')
        #print(ser)
        #ser = Serial(PORT, BAUDRATE, timeout=0.1)
        #print('ser\n')
        print("ser")
        #print('ser\n')
    except:
        continue
    finally:
        print('Successfully connected to Serial device')
        break

while True:
    timestamp = str(getTime())
    #line2 = ser.readline() #--> UNCOMMENT THIS
    #packet_counter += 1

    if packet_counter != MAX_COUNT:
        #line = line2.decode() #--> UNCOMMENT THIS
        line = "0.1233 PT1:100, PT2:200, PT3:300, PT4:400, PT5:900, PT6:600, LC1:100, LC2:200, S:0, I:0, F:0, V:0, D:0, Q:1, M:0, P:0" #--> COMMENT THIS
        line = line.replace(", S", "|S")


    
        '''    

        line = line.replace(",", "")
        # this starts the string at the first PT
        # the millis value that prints should be ignored
        index_of_p = line.find('P')
        line = line[index_of_p:]
        
        # Processing data
        line_processing = re.sub(r'[A-Z]+\d:', ',', line)
        line_processing = line_processing.replace(",", "", 1)
        line_processing = "".join(line_processing.split())
        data = line_processing.split(',')
        '''
        line = line.replace(",", "")
        print("LINE IS " + line)
        # Find the index of 'PT'
        index_of_p = line.find('P')
        line = line[index_of_p:]

        # Find the index of '|', and if not found, set it to the end of the line
        index_of_pipe = line.find('|')
        if index_of_pipe == -1:
           index_of_pipe = len(line)

        # Processing data until '|'
        line_processing = re.sub(r'[A-Z]+\d:', ',', line[:index_of_pipe-1])
        line_processing = line_processing.replace(",", "", 1)
        line_processing = "".join(line_processing.split())
        data = line_processing.split(',')
        print(data)
        
        index = line.find("|")
        opto_part = line[index_of_pipe+1:]  # Get everything after '|'
        opto_data = re.findall(r'\d+', opto_part)  # Find all numeric values using regex
        
        # Reset packet counter
        packet_counter = 0

        # Sending data to Grafana as:
        # pt1,pt2,pt3,pt4,pt5,pt6,lc1,lc2,tc1,tc2

        # combine all values together, separated by comma
        fields = ''
        for key, val in zip(field_keys, data):
            fields += f'{key}={val},'
        # delete comma after tc2
        fields = fields.strip(',')

        # create influx string
        influx_string = measurement + ' ' + fields + ' ' + timestamp
        print(influx_string)
        UDPClientSocket.sendto(influx_string.encode(), serverAddressPort)

        # combine all values together, separated by comma
        fields2 = ''
        for key2, val2 in zip(field_keys2, opto_data):
            fields2 += f'{key2}={val2},'
        # delete comma after last
        fields2 = fields2.strip(',')

        # create influx string
        influx_string2 = measurement2 + ' ' + fields2 + ' ' + timestamp
        print(influx_string2)
        UDPClientSocket.sendto(influx_string2.encode(), serverAddressPort2)

        time.sleep(0.24) #--> COMMENT THIS
