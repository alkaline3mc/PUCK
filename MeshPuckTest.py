import serial
import time


def hex_to_int(msb,lsb):
    return ord(msb) * 256 + ord(lsb)

print "Xbee Data Logger\n\r"
print "Initialize Data Log File"
timestr = time.strftime("%Y%m%d-%H%M%S")
fileName = "PUCLOG_" + timestr +".csv"
logFile = open(fileName,'w')
print "Data Log File created with name: ", fileName
logFile.write("Time,ID,X,Y,Z\r")
#______________________________________________________________________________Connect to Xbee
print "Initialize Xbee Serial Communication"
ser = serial.Serial('COM5', 115200) # Establish the connection on a specific port
#______________________________________________________________________________Begin Data Collection
print "Begin Data Collection"
#while True:
#    print ser.inWaiting()
#Packets are 27 bytes
count=0
while count < 100:
    if ser.inWaiting()==27:
        packet = ser.read(27)
        timeStamp = time.time()
        sourceID = hex_to_int(packet[10], packet[11])
        xVal = hex_to_int(packet[20],packet[21])
        yVal = hex_to_int(packet[22],packet[23])
        zVal = hex_to_int(packet[24],packet[25])
        logFile.write(str(timeStamp) + "," + str(sourceID) + "," + str(xVal) + "," + str(yVal) + "," + str(zVal) + "\r")
        count = count + 1
        print count
    elif ser.inWaiting()>27:
        print ser.inWaiting()
        ser.flushInput()
print "File Closed Out"
logFile.close()