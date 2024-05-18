#from serial import Serial
import socket
import time
import re
import os
import random

# MODIFY PORT AND BAUDRATE
# PORT = '/dev/cu.usbserial-0001' 
PORT = '/dev/cu.SLAB_USBtoUART' #--> UNCOMMENT THIS
# PORT = '/dev/cu.usbmodem131488301'
BAUDRATE = 115200
MAX_COUNT = -31 #######Change this

measurement = 'sensorvals'

field_keys = ["pt1", "pt2", "pt3", "pt4", "pt5", "pt6", "lc1", "lc2"]

# just in case
def getTime():
    return time.time_ns()

start_time = str(getTime())
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# DO I NEED TO CHANGE SERVER ADDRESS PORT????
serverAddressPort = ('127.0.0.1', 4001)
#ser = Serial(PORT, BAUDRATE, timeout=0.1)

# Modify value of N for varying smoothness
N = 3
packet_counter = 0

while True:
    try:
        print('ser\n')
        print(ser)
        #ser = Serial(PORT, BAUDRATE, timeout=0.1)
        print('ser\n')
        print(ser)
        print('ser\n')
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
        
        #line = "0.1233 PT1: 100, PT2: 200, PT3: 3000, PT4: 400, PT5: 500, PT6: 60, LC1: 0.1, LC2: 0.9" #--> COMMENT THIS
        line = "0.1233 PT1: 100, PT2: 200, PT3: 3000, PT4: 400, PT5: 500, PT6: 60, LC1: 0.1, LC2: " #--> COMMENT THIS
        line+=str(random.randint(0, 3))
        # Original string
        #line = "0.1233 PT1: 10000, PT2: 2000, PT3: 3000, PT4: 4000, PT5: 9000, PT6: 6000, LC1: 0.00001, LC2: 0.009"

    
            
        print(line)
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
        time.sleep(0.05)
