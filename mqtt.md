# MQTT interface

## topic: PoolHeater/get/current
descr: Data will be published on your MQTT server every n seconds using this topics:  
direction: read  
response: (json)  
```json
{
  "power": true,
  "mode": "heat",
  "temp_in": 21,
  "temp_out": 23,
  "temp_target": 28,
  "wifi_rssi": 88,
  "timestamp": 345455
}
```
## topic: PoolHeater/set/timestamp
descr: SECONDS SINCE JAN 01 1970. (UTC)
direction: write  
data(string): 1686765521

## topic: PoolHeater/set/power
direction: write  
data(string): 1 or 0

## topic: PoolHeater/set/mode
direction: write  
data(string): 'heat' or 'cool' or 'auto'

## topic: PoolHeater/set/target
direction: write  
data(string): 28



# the raw mode

## topic: PoolHeater/set/rawmode
direction: write  
data(string): 1 or 0 (default: 0)

## topic: PoolHeater/raw/recv
direction: read  
response(10 byte hexstring): d20c282d070da00c9cbd

## topic: PoolHeater/raw/send
direction: write  
data(10 byte hexstring): cc0c282d070da00c9cbd