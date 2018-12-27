#ifndef SERIAL_H_
#define SERIAL_H_

#include <hardware.h>

#define SERIAL_BUFF_SIZE 256
#define RF_WS_INBOUND_DELIMITER '!'
#define RF_WS_OTA_KEY 'O'
#define RF_WS_CMD_KEY 'C'
#define RF_WS_BURN_UUID_KEY 'U'

void serialConsole();

#endif /* SERIAL_H_ */