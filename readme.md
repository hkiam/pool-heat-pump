# ESP8268 Pool Heater MQTT bridge

This project connects the pool heat pump which use a controller board with label CC120A V6.3 via mqtt in my home automation environment.

![CC120A Board](./assets/board.jpg)



# Connection

You have to connect terminal connector ( 3 wires) from the CC120A Board to your ESP8266 by using a 5v  bidirectional level shifter. The 5v <-> 3.3v level shifter is mandatory because the esp8266 is not 5V tolerant, and the heatpump controller is not working with 3.3v.
!!!attention!!!, the powerline supplies 18V, so you need a step down converter to realize 5v. 

TODO: more infos and maybe  pictures of the schemantic and connections


# Background & Reversing
I have decoded the data using a picoscope oscilloscope und a salea logic analyzer clone. You can find my tools and more informations in the folder "investigation". 
The decoded signal has some similarities th the nec ir protocol. There is a special startsequence and the bits are encoded by pulse length modulation.


# MQTT
see mqtt.md

# Node-Red
Node-Red example to expose the mqtt interface of the heat pump as homekit-service to control the pool heat pump via an apple device and the home app.

```json
[
    {
        "id": "d1eb2eb66caa1862",
        "type": "homekit-service",
        "z": "5b25b43cda4fb3fe",
        "isParent": true,
        "hostType": "0",
        "bridge": "409001a1.3e7a78",
        "accessoryId": "",
        "parentService": "",
        "name": "Pool",
        "serviceName": "Thermostat",
        "topic": "",
        "filter": false,
        "manufacturer": "NRCHKB",
        "model": "1.5.0",
        "serialNo": "Default Serial Number",
        "firmwareRev": "1.5.0",
        "hardwareRev": "1.5.0",
        "softwareRev": "1.5.0",
        "cameraConfigVideoProcessor": "ffmpeg",
        "cameraConfigSource": "",
        "cameraConfigStillImageSource": "",
        "cameraConfigMaxStreams": 2,
        "cameraConfigMaxWidth": 1280,
        "cameraConfigMaxHeight": 720,
        "cameraConfigMaxFPS": 10,
        "cameraConfigMaxBitrate": 300,
        "cameraConfigVideoCodec": "libx264",
        "cameraConfigAudioCodec": "libfdk_aac",
        "cameraConfigAudio": false,
        "cameraConfigPacketSize": 1316,
        "cameraConfigVerticalFlip": false,
        "cameraConfigHorizontalFlip": false,
        "cameraConfigMapVideo": "0:0",
        "cameraConfigMapAudio": "0:1",
        "cameraConfigVideoFilter": "scale=1280:720",
        "cameraConfigAdditionalCommandLine": "-tune zerolatency",
        "cameraConfigDebug": false,
        "cameraConfigSnapshotOutput": "disabled",
        "cameraConfigInterfaceName": "",
        "characteristicProperties": "{\"TargetHeatingCoolingState\":{\"validValues\":[0,1]},\"CurrentHeatingCoolingState\":{\"validValues\":[0,1]}}",
        "waitForSetupMsg": false,
        "outputs": 2,
        "x": 830,
        "y": 720,
        "wires": [
            [
                "62263d416c7eba55"
            ],
            []
        ]
    },
    {
        "id": "3f112b1122c39ef0",
        "type": "mqtt in",
        "z": "5b25b43cda4fb3fe",
        "name": "",
        "topic": "PoolHeater/get/current",
        "qos": "2",
        "datatype": "auto-detect",
        "broker": "c525d14c5c4c5e1d",
        "nl": false,
        "rap": true,
        "rh": 0,
        "inputs": 0,
        "x": 400,
        "y": 720,
        "wires": [
            [
                "e1daa1e621b44f76",
                "6e198c1249e9a2ab"
            ]
        ]
    },
    {
        "id": "e1daa1e621b44f76",
        "type": "function",
        "z": "5b25b43cda4fb3fe",
        "name": "Übersetze von MQTT",
        "func": "\n/*\nTargetHeatingCoolingState:\n    Off\t0\n    Heat\t1\n    Cool\t2\n    Auto\t3\n\nCurrentHeatingCoolingState:\n    Off\t    0\n    Heat\t1\n    Cool\t2\n\n */\nvar currentHeatingState = 0;\nif (msg.payload.power ){\n    if (msg.payload.mode == 'cool'){\n        currentHeatingState = 2;\n    }else {\n        currentHeatingState = 1;\n    }\n}\n\nvar targetHeatingState = 0;\nif (msg.payload.power) {\n    if (msg.payload.mode == 'cool') {\n        targetHeatingState = 2;\n    }\n    else if (msg.payload.mode = 'auto') {\n        targetHeatingState = 1;\n    } \n    else {\n        targetHeatingState = 1;\n    }\n}\n\nmsg.payload = { \n    CurrentTemperature: msg.payload.temp_in,  \n    TargetTemperature: msg.payload.temp_target, \n    CurrentHeatingCoolingState: currentHeatingState, \n    TargetHeatingCoolingState: targetHeatingState \n}\n\nreturn msg;",
        "outputs": 1,
        "noerr": 0,
        "initialize": "",
        "finalize": "",
        "libs": [],
        "x": 640,
        "y": 720,
        "wires": [
            [
                "d1eb2eb66caa1862",
                "50487ecedd5a6332"
            ]
        ]
    },
    {
        "id": "eee0cc78758b70e5",
        "type": "function",
        "z": "5b25b43cda4fb3fe",
        "name": "Übersetze nach MQTT",
        "func": "//{\"TargetHeatingCoolingState\":0}\n//{\"TargetTemperature\":31}\n\nif (\"TargetHeatingCoolingState\" in msg.payload){\n    return { topic: 'PoolHeater/set/power', payload: msg.payload.TargetHeatingCoolingState!=0?1:0 }\n}\nif (\"TargetTemperature\" in msg.payload) {\n    return { topic: 'PoolHeater/set/target', payload: Math.floor(msg.payload.TargetTemperature) }\n}\n\nreturn;",
        "outputs": 1,
        "noerr": 0,
        "initialize": "",
        "finalize": "",
        "libs": [],
        "x": 1340,
        "y": 720,
        "wires": [
            [
                "7eaaa775d2e16d30"
            ]
        ]
    },
    {
        "id": "62263d416c7eba55",
        "type": "switch",
        "z": "5b25b43cda4fb3fe",
        "name": "Filtere nur, wenn durch Nutzer ausgelöst",
        "property": "hap.session",
        "propertyType": "msg",
        "rules": [
            {
                "t": "nempty"
            }
        ],
        "checkall": "true",
        "repair": false,
        "outputs": 1,
        "x": 1060,
        "y": 720,
        "wires": [
            [
                "eee0cc78758b70e5"
            ]
        ]
    },
    {
        "id": "7eaaa775d2e16d30",
        "type": "mqtt out",
        "z": "5b25b43cda4fb3fe",
        "name": "Test",
        "topic": "",
        "qos": "",
        "retain": "",
        "respTopic": "",
        "contentType": "",
        "userProps": "",
        "correl": "",
        "expiry": "",
        "broker": "c525d14c5c4c5e1d",
        "x": 1510,
        "y": 720,
        "wires": []
    },
    {
        "id": "6e198c1249e9a2ab",
        "type": "trigger",
        "z": "5b25b43cda4fb3fe",
        "name": "",
        "op1": "",
        "op2": "{\"CurrentHeatingCoolingState\":0,\"TargetHeatingCoolingState\":0}",
        "op1type": "nul",
        "op2type": "json",
        "duration": "120",
        "extend": false,
        "overrideDelay": false,
        "units": "s",
        "reset": "",
        "bytopic": "all",
        "topic": "topic",
        "outputs": 1,
        "x": 710,
        "y": 520,
        "wires": [
            [
                "d1eb2eb66caa1862",
                "996359c66cb78328"
            ]
        ]
    },
    {
        "id": "409001a1.3e7a78",
        "type": "homekit-bridge",
        "bridgeName": "Demo 1",
        "pinCode": "153-10-538",
        "port": "",
        "allowInsecureRequest": false,
        "manufacturer": "NRCHKB",
        "model": "Demo",
        "serialNo": "1.1.2",
        "firmwareRev": "",
        "hardwareRev": "",
        "softwareRev": "",
        "customMdnsConfig": false,
        "mdnsMulticast": true,
        "mdnsInterface": "",
        "mdnsPort": "",
        "mdnsIp": "",
        "mdnsTtl": "",
        "mdnsLoopback": true,
        "mdnsReuseAddr": true,
        "allowMessagePassthrough": true
    },
    {
        "id": "c525d14c5c4c5e1d",
        "type": "mqtt-broker",
        "name": "RybyNas MQTT",
        "broker": "192.168.178.56",
        "port": "1883",
        "clientid": "",
        "autoConnect": true,
        "usetls": false,
        "protocolVersion": "4",
        "keepalive": "60",
        "cleansession": true,
        "birthTopic": "",
        "birthQos": "0",
        "birthPayload": "",
        "birthMsg": {},
        "closeTopic": "",
        "closeQos": "0",
        "closePayload": "",
        "closeMsg": {},
        "willTopic": "",
        "willQos": "0",
        "willPayload": "",
        "willMsg": {},
        "userProps": "",
        "sessionExpiry": ""
    }
]
```
