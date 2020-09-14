import socket
import fcntl
import struct
import time
import paho.mqtt.client as mqtt
import paho.mqtt.subscribe as subscribe

def on_message(client, userdata, msg):
    global gv
    print( msg.topic+" "+str(msg.payload))
    gv = str(msg.payload)
    
counter = 0
client = mqtt.Client('SmartTV') #raspberry PI client name
client.connect("192.168.137.2", 1883, 60) #connect the google cloud MQTT Brocker
client.on_message = on_message

while client.loop() == 0:
        time.sleep(1)
        counter += 1
        client.subscribe("SmartWindow") #this is the topic name 
        
        if counter == 15: #1 hour output.txt save the value
          counter = 0
          length = len(gv)-1
          s = gv[2:length]
          f = open("output.txt", 'wt')
          f.write(s)
          f.close()
