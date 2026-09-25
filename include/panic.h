#ifndef PANIC_H
#define PANIC_H

/*
 * 内核致命错误处理
 * 设计思路：遇到不可恢复的错误时，关中断、打印原因、停机等外部重启。
 * 不做花哨的寄存器 dump，先能定位"哪挂了"就够了。
 */
void panic(const char *msg) __attribute__((noreturn));

#endif
