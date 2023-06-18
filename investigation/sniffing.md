Fehler EE3

Connected to MQTT Broker: 192.168.178.56
message received:  dd 18 1a 0a 00 19 00 00 00 55 
message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  dd 18 1a 0a 00 19 00 04 00 59 
message received:  dd 18 1a 0b 00 19 00 04 00 5a 
message received:  dd 18 1a 0d 00 19 00 04 00 5c 
message received:  dd 18 1b 10 00 19 00 04 00 60 
message received:  dd 18 1b 11 00 19 00 04 00 61 
message received:  dd 18 1c 13 00 19 00 04 00 64 
message received:  dd 18 1b 14 00 19 00 00 00 60 
message received:  dd 18 1b 15 00 18 00 00 00 60 
message received:  dd 18 1a 15 00 18 00 00 00 5f 
message received:  dd 18 19 15 00 18 00 00 00 5e 
message received:  dd 18 19 16 00 18 00 00 00 5f 
message received:  dd 18 18 16 00 18 00 00 00 5e


--------------


maik@MacBook-Pro-von-Maik-2 Pool Heat Pump % ls    
PWP             assets          coverage        investigation   mqtt.md         mqtttest        readme.md
maik@MacBook-Pro-von-Maik-2 Pool Heat Pump % cd investigation 
maik@MacBook-Pro-von-Maik-2 investigation % ls
Picoscope Decoder       Salea Decoder           decoder                 protocol.md
maik@MacBook-Pro-von-Maik-2 investigation % cd ..
maik@MacBook-Pro-von-Maik-2 Pool Heat Pump % ls
PWP             assets          coverage        investigation   mqtt.md         mqtttest        readme.md
maik@MacBook-Pro-von-Maik-2 Pool Heat Pump % cd mqtttest 
maik@MacBook-Pro-von-Maik-2 mqtttest % ls
tester.py
maik@MacBook-Pro-von-Maik-2 mqtttest % python3 ./tester.py 
Connected to MQTT Broker: 192.168.178.56
^CTraceback (most recent call last):
  File "/Users/maik/Cloud/Source/privat/Pool Heat Pump/mqtttest/./tester.py", line 36, in <module>
    client.loop_forever()
  File "/opt/homebrew/lib/python3.11/site-packages/paho/mqtt/client.py", line 1756, in loop_forever
    rc = self._loop(timeout)
         ^^^^^^^^^^^^^^^^^^^
  File "/opt/homebrew/lib/python3.11/site-packages/paho/mqtt/client.py", line 1150, in _loop
    socklist = select.select(rlist, wlist, [], timeout)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
KeyboardInterrupt

maik@MacBook-Pro-von-Maik-2 mqtttest % python3 ./tester.py
Connected to MQTT Broker: 192.168.178.56
message received:  dd18190a001900000054
message topic:  PoolHeater/raw/recv
message received:  d20c282d070da04c9cfd
message topic:  PoolHeater/raw/recv
message received:  dd181a0a001900000055
message topic:  PoolHeater/raw/recv
^Z
zsh: suspended  python3 ./tester.py
maik@MacBook-Pro-von-Maik-2 mqtttest % python3 ./tester.py
Connected to MQTT Broker: 192.168.178.56
message received:  dd181a0a001900000055
message topic:  PoolHeater/raw/recv
message received:  d20c282d070da04c9cfd
message topic:  PoolHeater/raw/recv
^CTraceback (most recent call last):
  File "/Users/maik/Cloud/Source/privat/Pool Heat Pump/mqtttest/./tester.py", line 38, in <module>
    client.loop_forever()
  File "/opt/homebrew/lib/python3.11/site-packages/paho/mqtt/client.py", line 1756, in loop_forever
    rc = self._loop(timeout)
         ^^^^^^^^^^^^^^^^^^^
  File "/opt/homebrew/lib/python3.11/site-packages/paho/mqtt/client.py", line 1150, in _loop
    socklist = select.select(rlist, wlist, [], timeout)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
KeyboardInterrupt

maik@MacBook-Pro-von-Maik-2 mqtttest % python3 ./tester.py
Connected to MQTT Broker: 192.168.178.56
message received:  d20c282d070da04c9cfd
message received:  dd181a0a001900000055
^CTraceback (most recent call last):
  File "/Users/maik/Cloud/Source/privat/Pool Heat Pump/mqtttest/./tester.py", line 38, in <module>
    client.loop_forever()
  File "/opt/homebrew/lib/python3.11/site-packages/paho/mqtt/client.py", line 1756, in loop_forever
    rc = self._loop(timeout)
         ^^^^^^^^^^^^^^^^^^^
  File "/opt/homebrew/lib/python3.11/site-packages/paho/mqtt/client.py", line 1150, in _loop
    socklist = select.select(rlist, wlist, [], timeout)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
KeyboardInterrupt

maik@MacBook-Pro-von-Maik-2 mqtttest % python3 ./tester.py
Connected to MQTT Broker: 192.168.178.56
message received:  dd 18 1a 0a 00 19 00 00 00 55 
message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  dd 18 1a 0a 00 19 00 04 00 59 
message received:  dd 18 1a 0b 00 19 00 04 00 5a 
message received:  dd 18 1a 0d 00 19 00 04 00 5c 
message received:  dd 18 1b 10 00 19 00 04 00 60 
message received:  dd 18 1b 11 00 19 00 04 00 61 
message received:  dd 18 1c 13 00 19 00 04 00 64 
message received:  dd 18 1b 14 00 19 00 00 00 60 
message received:  dd 18 1b 15 00 18 00 00 00 60 
message received:  dd 18 1a 15 00 18 00 00 00 5f 
message received:  dd 18 19 15 00 18 00 00 00 5e 
message received:  dd 18 19 16 00 18 00 00 00 5f 
message received:  dd 18 18 16 00 18 00 00 00 5e 
message received:  dd 18 18 17 00 18 00 00 00 5f 
message received:  dd 18 18 17 00 19 00 00 00 60 
message received:  dd 18 18 16 00 19 00 00 00 5f 
message received:  dd 18 19 16 00 19 00 00 00 60 
message received:  dd 18 19 07 00 19 00 00 00 51 
message received:  dd 18 19 06 00 19 00 00 00 50 
message received:  dd 18 19 08 00 19 00 00 00 52 
message received:  dd 18 1a 08 00 19 00 00 00 53 
message received:  dd 18 19 09 00 19 00 00 00 53 
message received:  dd 18 1a 09 00 19 00 00 00 54 
message received:  dd 19 1a 09 00 19 00 00 00 55 
message received:  dd 19 1a 0a 00 19 00 00 00 56 
message received:  dd 19 1a 09 00 18 00 00 00 54 
message received:  dd 18 1a 09 00 18 00 00 00 53 
message received:  dd 19 1a 0a 00 1a 00 00 00 57 

message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  cc 0c 28 2d 07 0d a0 0c 9c bd   <- Ausschalten
message received:  d2 0c 28 2d 07 0d a0 0c 9c bd 

message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  cc 0c 28 2d 07 0d a0 8c 9c 3d   <-  9 auf 1?
message received:  d2 0c 28 2d 07 0d a0 8c 9c 3d 


message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  cc 0c 28 2d 07 0d a0 cc 9c 7d   <- zusätzlich einschalten;  Mask "0b10000000" ist also parameter 9
message received:  d2 0c 28 2d 07 0d a0 cc 9c 7d 


message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  cc 0c 28 2d 07 0d a0 0e 9c bf 
message received:  dd 19 19 15 00 17 00 00 00 5e 

message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  cc 0c 28 2d 07 0d a0 2a 9c db 
message received:  cc 0c 28 2d 07 0d a0 08 9c b9 
message received:  d2 0c 28 2d 07 0d a0 08 9c b9 


message received:  d2 0c 28 2d 07 0d a0 4c 9c fd 
message received:  cc 0c 28 2d 07 0d a0 4c 9c fd 
message received:  dd 19 18 16 00 17 00 00 00 5e 




9 von 0 auf 1 und dann wieder auf 0
8 von 1 auf ? und dann wieder auf 1