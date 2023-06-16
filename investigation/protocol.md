
# Status Device
Pos Default Description
0   12      15-35   
1   40
2   45      Zykluszeit im Heizmodus
3   7       Zieltemp. um den Defrost-Modus zu starten defrost  -30 bis -7     
4   13      Zieltemp. um den Defrost-Modus zu beenden exit defrost   2-30
5   8       time exit defros 1-12   Zeit zum Austritt aus dem Defrost-Modus
6   1       Mode 0 Cool, 1 Head + Cool, 2 Head + Cool + Aux     
7   1       Expansion Valve : 1 Auto  ?
8   1       target super heat -15 - 15C default 3   ?
9   0
A   29      Wassertemperatur für den Automatischen Modus
B   18      inlet water temp    Wassereintrittstemperatur
C   19      outlet water temp   Wasseraustrittstemperatur
D   2       gas return temp?    Kondensiertemperatur bei Heizmodus
E   0       Condenser temperature under heating mode ?      Gas-Rücklauftemperatur
F   17      ambient?            Außentemperatur



# Signal
gnd
dat 5v
vcc 18v



# MSG DD
Temperatures ?
Direction: Device -> Terminal
dd 15 15 0d 00 11 00 00 00 48
|  |  |  |  |  |  |  |  |  |
|  |  |  |  |  |  |  |  |   - Checksum
|  |  |  |  |  |  |  |   -
|  |  |  |  |  |  |   -
|  |  |  |  |  |   -
|  |  |  |  |   - ambient
|  |  |  |   -
|  |  |   -
|  |   -   outlet water temp
|   - inlet water temp
 -


# MSG CA
Initial Msg, register Terminal?
Direction: Terminal -> Device
ca ca ca ca ca ca ca ca ca 50 
 
# MSG CC
set new state
Direction: Terminal -> Device
                   
cc 0c 1c 2d 07 0d a0 4c 9c f1
                     |   | |
                     |   |  - Checksum
                     |    - auto mode + Targettemperature 1c 
                      - On/Off, heat mode?

# MSG D2
Status Device
Direction: Device -> Terminal
see: msg cc
d2 0c 1c 2d 07 0d a0 4c 9c f1 




# Checksum
   XX XX XX XX XX XX XX XX CC
d2 0c 1c 2d 07 0d a0 0c 9c b1

CC = SUM(XX)&0xFF


# Samples

'''
d20c282d070da04c9cfd    auto on
d20c282d070da04c1c7d    cool on
d20c282d070da06c1c9d    heat on
d20c282d070da02c1c5d    heat off              
'''


start aus
dump: d2 0c 1c 2d 07 0d a0 0c 9c b1 
dump: dd 15 15 0f 00 11 00 00 00 4a 

button an  (21 grad)
                           X  XX
dump: cc 0c 1c 2d 07 0d a0 4c 9c f1 
dump: d2 0c 1c 2d 07 0d a0 4c 9c f1 

b on läuft
status:
dump: dd 15 15 0d 00 11 00 00 00 48 
dump: dd 15 16 0d 00 11 00 00 00 49 
dump: dd 15 16 01 00 11 00 00 00 3d 
dump: dd 15 16 00 00 11 00 00 00 3c 
dump: dd 15 16 01 00 10 00 00 00 3c 
dump: dd 15 16 02 00 11 00 00 00 3e 

ziel temp (29)
dump: cc 0c 1c 2d 07 0d a0 4c 9d f2 
dump: d2 0c 1c 2d 07 0d a0 4c 9d f2 

ziel temp (30)
dump: cc 0c 1c 2d 07 0d a0 4c 9e f3 
dump: dd 15 16 04 00 11 00 00 00 40 


ziel temp (31)
dump: cc 0c 1c 2d 07 0d a0 4c 9f f4 
dump: d2 0c 1c 2d 07 0d a0 4c 9f f4 

ziel temp (29, 28)
dump: cc 0c 1c 2d 07 0d a0 4c 9d f2 
dump: cc 0c 1c 2d 07 0d a0 4c 9c f1 
dump: d2 0c 1c 2d 07 0d a0 4c 9c f1 

button off
                           X
dump: cc 0c 1c 2d 07 0d a0 0c 9c b1 
dump: d2 0c 1c 2d 07 0d a0 0c 9c b1 

zieltemp 22
                              XX
dump: cc 0c 1c 2d 07 0d a0 4c 96 eb 
dump: d2 0c 1c 2d 07 0d a0 4c 96 eb 

      


Status
21 Grad
dd 15 15 0d 00 11 00 00 00 48

25 Grad?
dd 19 1a 05 00 15 00 00 00 4d : 2
dd 19 19 14 00 16 00 00 00 5c : 5
dd 19 19 13 00 16 00 00 00 5b : 2
dd 19 19 13 00 15 00 00 00 5a : 2
dd 19 19 10 00 15 00 00 00 57 : 1
dd 19 19 0f 00 15 00 00 00 56 : 1
dd 19 19 0d 00 15 00 00 00 54 : 1
dd 19 19 0c 00 15 00 00 00 53 : 1
dd 19 19 09 00 15 00 00 00 50 : 1

# some Manuals 

https://www.pooltotal.com/bilder/Waermepumpen/Installations-undBedienungsanleitungSmartWaermepumpe.pdf

https://www.poolomio.de/mediafiles/Handbuecher/Solar/Azuro-BP-85-140HS.pdf

https://www.kwad.at/wp-content/uploads/2019/03/W%C3%A4rmepumpe-11070110721107411076.pdf

https://www.albixon.de/data/files/manuals/germany/warmepumpen/schwimmbad-warmepumpe-inverboost-c-manual.pdf

https://www.saltmaster.at/uploads/ZGZClAIF/1567772_Green_Heat_Beschreibung.pdf

http://lksshop.de/download/Bauanleitung_Waermepumpe.pdf

https://www.schwimmbadbau24.de/mediafiles/PDF/brilix/waermepumpe_thp_brilix_anleitung.pdf