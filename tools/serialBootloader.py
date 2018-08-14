import sys, getopt, serial, binascii

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

    print("Initializing the device ..")
    for line in lines:
        length = len(line)
        if length > 11:
            if line[0] == ':':             
                print line[9:length-2]
                ser.write(line[9:length-2])

if __name__ == "__main__":
    main(sys.argv[1:])

