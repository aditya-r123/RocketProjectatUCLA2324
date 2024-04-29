import time
import socket
import random

# not used in serial version since arduino is what sends the data
def getTime():
    return time.time_ns()

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Initialize stuff
measurement = 'sensorvals'
field_keys = ["pt1", "pt2", "pt3", "pt4", "pt5", "pt6", "lc1", "lc2"]
num_fields = len(field_keys)
field_values = [100 for i in range(num_fields)]
serverAddressPort1 = ('127.0.0.1', 4001)

serverAddressPort2 = ('127.0.0.1', 65415)
# Initialize stuff
measurement2 = 'octovals'
field_keys2 = ["purge", "dump", "vent", "fill", "qd", "ignite", "mpv", "siren"]
num_fields2 = len(field_keys2)
field_values2 = [100 for i in range(num_fields2)]

N = 3
while True:
    lc_avgs = []
    for i in range(N):
        # Update data (we'd read from serial here)
        timestamp = getTime()
        for i in range(num_fields):
            field_values[i] = random.gauss(10, 500)
        
        # update field string
        fields = ''
        for i in range(num_fields):
            fields += f'{field_keys[i]}={field_values[i]}'
            
            if i != num_fields-1:
                fields += ','
        
        # influx string to send to telegraf
        data = measurement + ' ' + fields + ' ' + str(timestamp)
        print(data.encode())
        #time.sleep(0.35)
        UDPClientSocket.sendto(data.encode(), serverAddressPort1)

        # Update data (we'd read from serial here)
        timestamp2 = getTime()
        for i in range(num_fields2):
            field_values2[i] = random.randint(0, 1)
        
        # update field string
        fields2 = ''
        for i in range(num_fields2):
            fields2 += f'{field_keys2[i]}={field_values2[i]}'
            
            if i != num_fields2-1:
                fields2 += ','
        
        # influx string to send to telegraf
        data2 = measurement2 + ' ' + fields2 + ' ' + str(timestamp2)
        print(data2.encode())
        time.sleep(0.35)
        UDPClientSocket.sendto(data2.encode(), serverAddressPort2)
   
       
        
