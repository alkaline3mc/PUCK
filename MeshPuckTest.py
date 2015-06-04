import serial
import time
from xbee import XBee,ZigBee


accel=0
ser = serial.Serial('COM5', 9600) # Establish the connection on a specific port
xbee = ZigBee(ser)

print "Start"
while True: 
    if ser.inWaiting() == 23:
        packet = ser.read(23)
        pStart = packet[0]
        pSizeMSB = packet[1]*256
        pSizeLSB = packet[2]
        pSize = pSizeMSB + pSizeLSB
        """print "Start: ", hex(ord(packet[0])),\
              ", SizeMSB: ", hex(ord(packet[1])),\
              ", SizeLSB: ", hex(ord(packet[2])),\
              ", apiID: ", hex(ord(packet[3]))
        print "sAddress(1): ", hex(ord(packet[4])),\
              "sAddress(2): ", hex(ord(packet[5])),\
              "sAddress(3): ", hex(ord(packet[6])),\
              "sAddress(4): ", hex(ord(packet[7])),\
              "sAddress(5): ", hex(ord(packet[8])),\
              "sAddress(6): ", hex(ord(packet[9])),\
              "sAddress(7): ", hex(ord(packet[10])),\
              "sAddress(8): ", hex(ord(packet[11]))"""
        """for i in xrange (23):
            print hex(ord(packet[i]))"""
        print hex(ord(packet[0])),hex(ord(packet[1])),hex(ord(packet[2])),hex(ord(packet[3])),hex(ord(packet[4])),hex(ord(packet[5])),hex(ord(packet[6])),hex(ord(packet[7])),hex(ord(packet[8])),hex(ord(packet[9])),hex(ord(packet[10])),hex(ord(packet[11])),hex(ord(packet[12])),hex(ord(packet[13])),hex(ord(packet[14])),hex(ord(packet[15])),hex(ord(packet[16])),hex(ord(packet[17])),hex(ord(packet[18])),hex(ord(packet[19])),hex(ord(packet[20])),hex(ord(packet[21])),hex(ord(packet[22])),"\n\r"""
        #Use ID bytes 8-11 for the ID (follow up with Xbee as to why exactly the MSB are wrong ie get a reference sheet)
