#ifndef OPTIBOOT_H_
#define OPTIBOOT_H_

#ifndef APP_START_ADDR
#define APP_START_ADDR 0x00008000
#endif /* APP_START_ADDR */

#define IMAGE_PRESENT_ADDR ( APP_START_ADDR - 0x100 )

void setImageKey();
int  checkImageKey();
void startApplication();

#endif /* OPTIBOOT_H_ */
