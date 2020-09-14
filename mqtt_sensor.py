import paho.mqtt.client as mqtt
import fcntl
import struct
import time
from datetime import datetime




mqttc = mqtt.Client('RaspberryPI')
mqttc.connect("35.200.52.115", 1883) # 구글 클라우드에 실행되고 있는 MQTT 브로커에 연결

while mqttc.loop() == 0:
    now = datetime.now()
                   
    #print(now.minute)
    f = open("output.txt", 'rt') #센서 어플리케이션에서 출력한 txt 파일을 열어 값을 가져옴
    value = str(f.read())
    f.close()
        
    mqttc.publish('SmartWindow', value) #topic 이름 SmartWindow로 값을 전달
    print("success publish!")
    print(value)
    time.sleep(5)
        
    
        