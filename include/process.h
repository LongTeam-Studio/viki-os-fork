// include/process.h
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define TASK_RUNNING 0
#define TASK_READY   1
#define TASK_BLOCKED 2
#define TASK_ZOMBIE  3

typedef struct process {
    uint32_t pid;                // 进程ID
    uint32_t state;              // 进程状态
    uint32_t esp;                // 保存的栈指针 (上下文切换的关键)
    uint32_t cr3;                // 页目录物理地址
    struct process *next;        // 链表指针
    char name[32];
    uint32_t kstack_top;         // ring0 内核栈顶
} process_t;

void process_init(void);
process_t *process_create(const char *name, void (*entry)(void));
process_t *process_create_user(const char *name, void (*entry)(void), uint32_t user_stack_top);
void schedule(void);
void switch_to(process_t *prev, process_t *next);
void process_exit(void);

extern process_t *current_process;

#endif
