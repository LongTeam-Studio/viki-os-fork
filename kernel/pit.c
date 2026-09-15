#include "../include/pit.h"
#include "../include/port.h"
#include "../include/interrupt.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

void pit_init(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;

    // 发送命令：通道0，先低后高字节，模式3(方波)，二进制
    outb(PIT_COMMAND, 0x36);
    // 写入分频值
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
}

// 在中断处理中调用，记录系统运行时间
static volatile uint64_t system_ticks = 0;
void timer_handler(void) {
    system_ticks++;
}
