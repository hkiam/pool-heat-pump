'''
d20c282d070da04c9cfd    auto on
d20c282d070da04c1c7d    cool on
d20c282d070da06c1c9d    heat on
d20c282d070da02c1c5d    heat off
              X X
'''

import paho.mqtt.client as mqtt

TOPIC = "PoolHeater/raw/recv"
BROKER_ADDRESS = "192.168.178.56"
PORT = 1883

cache = []

def on_message(client, userdata, message):
    msg = str(message.payload.decode("utf-8"))

    if msg not in cache:
        cache.append(msg)
        print("message received: ", msg)
        print("message topic: ", message.topic)

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker: " + BROKER_ADDRESS)
    client.subscribe(TOPIC)

if __name__ == "__main__":
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_ADDRESS, PORT)

    client.loop_forever()