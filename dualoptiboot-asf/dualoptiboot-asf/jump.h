#ifndef JUMP_H_
#define JUMP_H_

#define APP_START_ADDR 0x00004000

#define ROMSIZE APP_START_ADDR

#define INVALID_APP 0xFFFFFFFF

#include <stdbool.h>

void jump();
bool check_for_application();

#endif /* JUMP_H_ */
