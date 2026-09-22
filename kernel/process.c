#include "../include/process.h"
#include "../include/pmm.h"
#include "../include/vga.h"

/* 内部 helper：不依赖外部 libc，freestanding 环境自带 */
static void mem_zero(void *dst, uint32_t n) {
    uint8_t *d = (uint8_t *)dst;
    while (n--) *d++ = 0;
}

static void str_copy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}

/*
 * 就绪队列
 * 设计思路：单链表 + 头尾指针，O(1) 入队、O(1) 出队。
 * 调度器要按时间片轮转，队列顺序即执行顺序。
 */
static process_t *ready_queue_head = 0;
static process_t *ready_queue_tail = 0;

/* 当前运行的进程。初始指向 boot_process，让第一次调度前有"上一个"可记录。 */
process_t *current_process = 0;

/* 占位进程：process_init 之后、第一个真进程创建之前充当 current_process */
static process_t boot_process;

static uint32_t next_pid = 1;

void process_init(void) {
    ready_queue_head = 0;
    ready_queue_tail = 0;
    next_pid = 1;

    /* 清空占位进程，把它当作"boot 自己" */
    mem_zero(&boot_process, sizeof(boot_process));
    boot_process.pid = 0;
    boot_process.state = TASK_RUNNING;
    str_copy(boot_process.name, "boot");
    current_process = &boot_process;
}

/*
 * 创建内核线程
 *
 * 栈帧布局（从高地址往低地址）：
 *   [栈顶]
 *   entry      <- ret 地址，将来 switch_to 的 ret 跳到这里
 *   0          <- ebp
 *   0          <- ebx
 *   0          <- esi
 *   0          <- edi   <- esp 指向这里
 *
 * 本 PR 只负责"造出这个栈"和"塞进队列"，不负责真正切过去。
 * 实际切换（isr.S 保存/恢复 esp）由后续 PR 完成。
 */
process_t *process_create(const char *name, void (*entry)(void)) {
    process_t *proc = (process_t *)pmm_alloc_page();
    if (!proc) return 0;
    mem_zero(proc, sizeof(process_t));

    proc->pid = next_pid++;
    str_copy(proc->name, name);
    proc->state = TASK_READY;
    proc->cr3 = 0;
    proc->kstack_top = 0;  /* 内核线程暂不需要单独的 ring0 栈 */

    /* 分配一页内核栈 */
    uint32_t *stack = (uint32_t *)pmm_alloc_page();
    if (!stack) return 0;
    uint32_t *sp = (uint32_t *)((uint32_t)stack + 4096);

    /* 手工压一个简化的 ret 帧，供将来的 switch_to 使用 */
    *(--sp) = (uint32_t)entry;  /* ret 地址 */
    *(--sp) = 0;                /* ebp */
    *(--sp) = 0;                /* ebx */
    *(--sp) = 0;                /* esi */
    *(--sp) = 0;                /* edi */

    proc->esp = (uint32_t)sp;

    /* 入队 */
    proc->next = 0;
    if (ready_queue_tail) {
        ready_queue_tail->next = proc;
    } else {
        ready_queue_head = proc;
    }
    ready_queue_tail = proc;

    vga_printf("process_create: pid=%u name=%s esp=0x%x\n",
               proc->pid, proc->name, proc->esp);

    return proc;
}
