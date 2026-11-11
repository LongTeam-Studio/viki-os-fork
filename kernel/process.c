#include "../include/process.h"
#include "../include/pmm.h"
#include "../include/string.h"
#include "../include/tss.h"
#include "../include/vga.h"

static process_t *ready_queue_head = 0;
static process_t *ready_queue_tail = 0;
process_t *current_process = 0;
static uint32_t next_pid = 1;

static process_t boot_process;   /* 占位，第一次调度前作为 current_process */

void process_init(void) {
    ready_queue_head = 0;
    ready_queue_tail = 0;
    next_pid = 1;

    memset(&boot_process, 0, sizeof(boot_process));
    boot_process.pid = 0;
    boot_process.state = TASK_RUNNING;
    strcpy(boot_process.name, "boot");
    current_process = &boot_process;
}

process_t *process_create(const char *name, void (*entry)(void)) {
    process_t *proc = (process_t *)pmm_alloc_page();
    if (!proc) return 0;
    memset(proc, 0, sizeof(process_t));

    proc->pid = next_pid++;
    strcpy(proc->name, name);
    proc->state = TASK_READY;
    proc->cr3 = 0;

    uint32_t *stack = (uint32_t *)pmm_alloc_page();
    if (!stack) return 0;
    uint32_t stack_top = (uint32_t)stack + 4096;

    uint32_t *sp = (uint32_t *)stack_top;

    /* 构造中断返回栈（RESTORE_ALL 按此顺序弹出）：
     *   [pt_regs_ptr]   ← addl $4 跳过
     *   edi..eax        ← popa（8 个）
     *   gs,fs,es,ds     ← 4 个
     *   int_no,err_code ← addl $8 跳过
     *   eip,cs,eflags   ← iret
     */
    *(--sp) = 0x202;              /* eflags: IF=1 */
    *(--sp) = 0x08;               /* cs: 内核代码段 */
    *(--sp) = (uint32_t)entry;    /* eip: 入口 */
    *(--sp) = 0;                  /* err_code */
    *(--sp) = 0;                  /* int_no */
    *(--sp) = 0x10;               /* ds */
    *(--sp) = 0x10;               /* es */
    *(--sp) = 0x10;               /* fs */
    *(--sp) = 0x10;               /* gs */
    *(--sp) = 0;                  /* eax */
    *(--sp) = 0;                  /* ecx */
    *(--sp) = 0;                  /* edx */
    *(--sp) = 0;                  /* ebx */
    *(--sp) = 0;                  /* esp (pusha 槽位，忽略) */
    *(--sp) = 0;                  /* ebp */
    *(--sp) = 0;                  /* esi */
    *(--sp) = 0;                  /* edi */
    *(--sp) = 0;                  /* pt_regs_ptr 占位 */

    proc->esp = (uint32_t)sp;

    proc->next = 0;
    if (ready_queue_tail) {
        ready_queue_tail->next = proc;
    } else {
        ready_queue_head = proc;
    }
    ready_queue_tail = proc;

    return proc;
}

void schedule(void) {
    if (!ready_queue_head) return;

    if (current_process && current_process->state == TASK_RUNNING) {
        current_process->state = TASK_READY;
        ready_queue_tail->next = current_process;
        ready_queue_tail = current_process;
        current_process->next = 0;
    }

    process_t *next = ready_queue_head;
    ready_queue_head = next->next;
    if (!ready_queue_head) ready_queue_tail = 0;

    next->state = TASK_RUNNING;
    process_t *prev = current_process;
    current_process = next;

    if (next->cr3 && prev->cr3 && next->cr3 != prev->cr3) {
        __asm__ volatile("mov %0, %%cr3" :: "r"(next->cr3));
    }

    /* 切到用户进程前，更新 TSS.esp0 指向它的 ring0 内核栈 */
    if (next->kstack_top) {
        tss_set_esp0(next->kstack_top);
    }
    /* switch_to 已废弃：中断返回路径会从 current_process->esp 恢复 */
}


process_t *process_create_user(const char *name, void (*entry)(void),
                               uint32_t user_stack_top) {
    process_t *proc = (process_t *)pmm_alloc_page();
    if (!proc) return 0;
    memset(proc, 0, sizeof(process_t));

    proc->pid = next_pid++;
    strcpy(proc->name, name);
    proc->state = TASK_READY;
    proc->cr3 = 0;

    /* ring0 内核栈：ring3 -> ring0 时 CPU 从 TSS.esp0 加载 */
    uint32_t *kstack = (uint32_t *)pmm_alloc_page();
    if (!kstack) return 0;
    proc->kstack_top = (uint32_t)kstack + 4096;

    /* 手工构造中断栈，与 SAVE_ALL + iret(5 字段) 布局一致 */
    uint32_t *sp = (uint32_t *)proc->kstack_top;

    *(--sp) = 0x23;               /* ss: 用户数据段 | RPL3 */
    *(--sp) = user_stack_top;     /* useresp */
    *(--sp) = 0x202;              /* eflags: IF=1 */
    *(--sp) = 0x1B;               /* cs: 用户代码段 | RPL3 */
    *(--sp) = (uint32_t)entry;    /* eip */
    *(--sp) = 0;                  /* err_code */
    *(--sp) = 0;                  /* int_no */
    *(--sp) = 0x23;               /* ds */
    *(--sp) = 0x23;               /* es */
    *(--sp) = 0x23;               /* fs */
    *(--sp) = 0x23;               /* gs */
    *(--sp) = 0;                  /* eax */
    *(--sp) = 0;                  /* ecx */
    *(--sp) = 0;                  /* edx */
    *(--sp) = 0;                  /* ebx */
    *(--sp) = 0;                  /* esp (pusha 槽位) */
    *(--sp) = 0;                  /* ebp */
    *(--sp) = 0;                  /* esi */
    *(--sp) = 0;                  /* edi */
    *(--sp) = 0;                  /* pt_regs_ptr 占位 */

    proc->esp = (uint32_t)sp;

    proc->next = 0;
    if (ready_queue_tail) {
        ready_queue_tail->next = proc;
    } else {
        ready_queue_head = proc;
    }
    ready_queue_tail = proc;

    return proc;
}

void process_exit(void) {
    while (1) { __asm__ volatile ("hlt"); }
}

