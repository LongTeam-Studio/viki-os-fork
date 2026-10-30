#include "../include/keyboard.h"
#include "../include/port.h"

#define KBD_DATA 0x60

#define BUF_SIZE 128
static volatile char buf[BUF_SIZE];
static volatile int head = 0, tail = 0;
volatile uint32_t kbd_irq_count = 0;
volatile uint8_t  kbd_last_sc   = 0;

/* 扫描码集 1，按下时的 ASCII */
static const char keymap[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',
    0,  '*', 0,  ' ',
};

void keyboard_irq(void) {
    kbd_irq_count++;
    uint8_t sc = inb(KBD_DATA);
    kbd_last_sc = sc;
    if (sc & 0x80) return;              /* 松键忽略 */
    if (sc >= 128) return;
    char c = keymap[sc];
    if (!c) return;
    int next = (head + 1) % BUF_SIZE;
    if (next == tail) return;           /* 满，丢 */
    buf[head] = c;
    head = next;
}

int keyboard_getchar(void) {
    if (head == tail) return -1;
    char c = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    return c;
}

void keyboard_init(void) {
    head = tail = 0;
}
