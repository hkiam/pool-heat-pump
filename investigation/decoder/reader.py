import csv


#see https://medium.com/blueeast/nec-transmission-protocol-for-air-conditioners-67c8a698e633
#https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
#first gap:     9.05215994
#second gap:    4.50559998
#third gap:     1.1468799899999995
#min gap:       0.8601599900000005

#1/0.000104 
#1/(0.860/1000) 
#1/(1.14/1000) 
def reverse_bit(num):
    result = 0
    for i in range(8):
        result = (result << 1) + (num & 1)
        num >>= 1
    return result


'''
0 no 
1 error
2 warn
3 debug 
'''
debuglevel = 1    
data2filename = {}

def analyseFile(filename):
    data = []
    bytedata = []
    spaces = []
    with open(filename, 'r') as file:
        # CSV-Datei als CSV-Objekt lesen
        
        # Durch jede Zeile der CSV-Datei iterieren
        lasttime = 0
        laststate = False
        statetime = None
        infos = []
        index = 0
        for line in file.readlines():
            try:
                zeile = line.split(" ")
                duration = float(zeile[0])/1000.0
                value = float(zeile[1])

                infos.append([duration, value, index])
                index = index+1;
            except:
                if debuglevel >= 2:
                    print("ignore line: %s" % zeile )
        
        nec = True
        if nec:
            span = 0.5
            leading_pulse_length = 9
            leading_space_length = 4.5
            #pause_length = leading_pulse_length * 2
            pause_length = leading_pulse_length * 0.7

            pulse_length = 1.15
            space_length = 4.5

            #0.86
            necstate = "wait_leading_pulse"
            bitbuffer = 1

            for idx,c in enumerate(infos):
                duartion = round(c[0],2)
                value = c[1]

                if duartion > pause_length:
                    spaces.append(c[0])
                    d1 = ''.join('{:02x}'.format(x) for x in bytedata)
                    print(d1)
                    if d1 in data2filename:
                        data2filename[d1]=data2filename[d1]+1
                    else:
                        data2filename[d1]=1

                    bytedata = []


                    if debuglevel >= 1:
                        if bitbuffer != 1:
                            print("error %s : %s" % (bin(bitbuffer),necstate)) 
                    if debuglevel >= 3:
                        print("pause found")
                    bitbuffer = 1
                    necstate = "wait_leading_pulse"

                if necstate == "wait_leading_pulse":
                    if abs(duartion-leading_pulse_length)<span:
                        necstate = "wait_leading_space"
                        print("%f" %  infos[idx-1][0])
                elif necstate == "wait_leading_space":
                    if abs(duartion-leading_space_length)<span:
                        necstate = "wait_pulse"
                elif necstate == "wait_pulse":               
                    if value == 0:
                        necstate = "wait_bit"
                    else:
                        if debuglevel >= 1:
                            print("error invalid pulse")
                elif necstate == "wait_bit":
                    #print("%f %d" % (duartion,value))
                    if value == 0:
                        if debuglevel >= 1:
                            print("error invalid bit")
                    bit = 0
                    if duartion > 2:
                        bit = 1
                    bitbuffer = (bitbuffer<<1) | bit

                    if bitbuffer & 0b100000000:
                        byte = reverse_bit(bitbuffer&0xFF)
                        bytedata.append(byte)
                        bitbuffer = 1
                    necstate = "wait_pulse"
            if bitbuffer != 1:
                if debuglevel >= 1:
                    print("error %s : %s" % (bin(bitbuffer),necstate)) 
        
        return bytedata


filename = 'records/dump.txt.1'
#filename = 'records/dump.txt.3'
#filename = '/Users/maik1/Waveforms/20230527-0005'

#filename =  '/Users/maik1/Waveforms/20230527-0006'
#filename = '/Users/maik1/Waveforms/20230527-0005/20230527-0005_26.csv'
#filename = '20230527-0004-14.csv'
#filename = '20230527-0003-12.csv'
#filename = '20230527-0002-10.csv'
#filename = '20230527-0001-8.csv'
#filename = "20230525-0004.csv"

import os


if os.path.isdir(filename):
    for f in os.listdir(filename):
        if f.startswith("."):
            continue
        fullfname = os.path.join(filename,f)
        if os.path.isfile(fullfname):
            print(f)
            data = analyseFile(fullfname)
            print()
            d1 = ''.join('{:02x}'.format(x) for x in data)
            if d1 in data2filename:
                data2filename[d1].append(f)
            else:
                data2filename[d1]=[f]
    for e in data2filename:
        print("%s : %s" % (e, data2filename[e] ))

elif os.path.isfile(filename):
    analyseFile(filename)
    keys = list(data2filename.keys())
    keys.sort()
    for e in keys:
        print("%s : %s" % (e, data2filename[e] ))
    print()
    for e in keys:
        if e.startswith("2"):
            print("%s : %s" % (' '.join(format(x, '02x') for x in list(map(lambda a: a ^ 0xFF, bytearray.fromhex(e)))), data2filename[e] ))
        else: 
            print("%s : %s" % (' '.join(format(x, '02x') for x in bytearray.fromhex(e)), data2filename[e]))
    

