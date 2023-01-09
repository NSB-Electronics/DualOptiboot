#ifndef JUMP_H_
#define JUMP_H_

#if defined( USB_SERIAL ) && defined( TEST_PRGM )
#define APP_START_ADDR 0x00020000
#elif defined( USB_SERIAL ) && !defined( TEST_PRGM )
#define APP_START_ADDR 0x00008000
#else
#define APP_START_ADDR 0x00002000
#endif

#define ROMSIZE APP_START_ADDR

void jump();

#endif /* JUMP_H_ */
