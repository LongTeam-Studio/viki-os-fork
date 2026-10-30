#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
int  keyboard_getchar(void);   /* 无字符返回 -1 */
void keyboard_irq(void);

extern volatile uint32_t kbd_irq_count;
extern volatile uint8_t  kbd_last_sc;

#endif
