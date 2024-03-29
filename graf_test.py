from serial import Serial
import socket
import time
import re

#MODIFY PORT AND BAUDRATE 
#PORT = '/dev/cu.usbserial-0001'
PORT = '/dev/cu.SLAB_USBtoUART'
#PORT = '/dev/cu.usbmodem131488301'
BAUDRATE = 9600

measurement = 'sensorvals'

field_keys = ["pt1", "pt2", "pt3", "pt4", "pt5", "pt6", "lc1", "lc2", "tc1", "tc2"]


#just in case
def getTime():
    return time.time_ns()
start_time = str(getTime())
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
#DO I NEED TO CHANGE SERVER ADDRESS PORT????
serverAddressPort = ('127.0.0.1', 4000)
ser = Serial(PORT, BAUDRATE, timeout=0.1)
while True:
    try:
        print('ser\n')
        print(ser)
        ser = Serial(PORT, BAUDRATE, timeout=0.1)
        print('ser\n')
        print(ser)
        print('ser\n')
    except:
        continue
    finally:
        print('Successfully connected to Serial device')
        break

#Modify value of N for varying smoothness
N = 3

while True:
    for i in range(N):
        timestamp = str(getTime())
        print(ser.readline())
        line = ser.readline().decode()
        print(line)
        #line = "0.1233 PT1: 100, PT2: 200, PT3: 300, PT4: 400, PT5: 500, PT6: 600, LC1: 0.1, LC2: 0.2, TC1: 1, TC2: 2"

        line = line.replace(",", "")
        #this starts the string at the first PT
        #the millis value that prints should be ignored
        index_of_p = line.find('P') 
        line = line[index_of_p:]
        # Processing data
        line_processing = re.sub(r'[A-Z]+\d:', ',', line)
        line_processing = line_processing.replace(",", "", 1)
        line_processing = "".join(line_processing.split())
        data = line_processing.split(',')



        #Sending data to Grafana as:
        #pt1,pt2,pt3,pt4,pt5,pt6,lc1,lc2,tc1,tc2
        
        #combine all values together, seperated by comma
        fields = ''
        for key,val in zip(field_keys, data):
            fields += f'{key}={val},'
        #delete comma after tc2
        fields = fields.strip(',')

        # create influx string
        influx_string = measurement + ' ' + fields + ' ' + timestamp
        #print(influx_string)
        UDPClientSocket.sendto(influx_string.encode(), serverAddressPort)
