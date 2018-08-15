import sys, getopt, serial, binascii

def getRecordLength(line):
    return line[1:3]

def getRecordAddr(line):
    return line[3:7]

def getRecordType(line):
    return line[7:9]

def getRecordPayload(line, length):
    return line[9:length-2]

def waitForAck(ser):
    byt = ser.read(1)
    print 'Got', byt
    if byt == '1':
        return True
    else:
        return False

def sendRecord(rType, rAddr, rLen, rPayload, ser):
    # Send the record type and wait for ack
    print 'Send type'
    ser.write(rType)
    if waitForAck(ser):
        # Send the record length
        print 'Send length'
        ser.write(rLen)
        if waitForAck(ser):
            # Send a payload if there is one
            if rLen != '00':
                print 'Send payload'
                ser.write(rPayload)
                if waitForAck(ser):
                    return True
            else:
                return True
        else:
            return False
    else:
        return False

def sendLine(line , ser):
    length = len(line)
    if length >= 11:
        if line[0] == ':':
            # Pull all values from the hex record
            recordLen = getRecordLength(line)
            recordAddr = getRecordAddr(line)
            recordType = getRecordType(line)
            recordPayload = getRecordPayload(line, length)

            # Uncomment these if you want to print record deatils
            print 'Record Lengh: ', recordLen
            print 'Record Addr: ', recordAddr
            print 'Record Type: ', recordType
            print 'Record Payload: ', recordPayload

            # Don't send an invalid record
            recInt = int(recordType)
            if recInt > 5:
                print 'Not a valid record type'
                return

            ok = sendRecord(recordType, recordAddr, recordLen, recordPayload, ser)
            if not ok:
                print 'failed to send, re trying'
                ok = sendRecord(recordType, recordAddr, recordLen, recordPayload, ser)
                if not ok:
                    print 'failed re try, exiting'
                    sys.exit(2)

            #print line[9:length-2]
            #ser.write(line[1:3])
            #ser.write(line[9:length-2])
            #ack = ser.read(1)
        else:
            print 'Not a valid hex record'

def main(argv):
    binaryFile = ''
    comPort = ''
    baudRate = 38400

    ## Check the args
    try:
        opts, args = getopt.getopt(argv,"hp:f:b:",["port=","hexFile=","baud="])
    except getopt.GetoptError:
        print 'serialBootloader.py -p <port> -f <hexFile> -b <baud>'
        sys.exit(2)
    
    ## Parse the args
    for opt, arg in opts:
        if opt == '-h':
            print 'serialBootloader.py -p <port> -f <hexFile> -b <baud>'
            sys.exit()
        elif opt in ("-p", "--port"):
            comPort = arg
        elif opt in ("-f", "--hexFile"):
            binaryFile = arg
        elif opt in ("-b", "--baud"):
            baudRate = arg
    
    ## Open the serial port
    ser = serial.Serial(
        port=comPort,
        baudrate=baudRate,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        xonxoff=0,
        rtscts=False,
        dsrdtr=False
    )

    with open(binaryFile) as f:
        lines = [line.rstrip('\n') for line in open(binaryFile)]

    ser.close()
    ser.open()
    ser.isOpen()

    ## Pass the hex file
    for line in lines:
        sendLine(line, ser)

if __name__ == "__main__":
    main(sys.argv[1:])

