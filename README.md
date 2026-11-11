## VIKI-OS

### 项目介绍

**VIKI-OS 一个借助AI编程辅助工具编写的开源操作系统**

### 运行环境

Ubuntu 18.04+ 64位、Debian 10+ 64位, qemu+kvm

### 运行方法
```bash
# 构建
make
# 运行
make run_qemu

# vscode  C/C++ debug
make run_debug_qemu 

# qemu 串口输出日志 主要用于AI自验证
make run_qemu_serial

```

### 如果你感兴趣也想要加入，请保存一下规则
#### 请使用AI coding
1. 为了项目统一规范，尽可能在你使用的AI工具中保持`AGENTS.md`定义
2. 每个功能的完成后，将相关`提示词`放到`user_prompt/prompt.md`中
3. 请保证**一个完整功能一个commit/pr**
4. AI coding之后必须要经过构建调试

#### 构建要求
1. 优先保证x64架构linux平台可以顺利构建运行
2. 默认采用gcc构建系统，若想要支持其他编译系统支持，请做兼容性支持

#### 其他架构
1. 目前只有支持x86，若想要支持aarch64、risc-v等架构，最后保持独立的分支

### TODO 清单

#### 基础层（已完成）

- [x] Multiboot2/GRUB 引导：高半核启动流程，物理 1MB 加载内核，设置初始页表后跳转高虚拟地址（`boot/boot.S`、`kernel/boot_paging.c`、`linker.ld`）
- [x] VGA 文本输出 + kprintf：清屏、光标管理、滚屏，支持 `%c`/`%s`/`%d`/`%u`/`%x`/`%X`/`%p`/`%%` 及宽度填充，手动实现 va_list（`kernel/vga.c`、`include/vga.h`）
- [x] GDT 全局描述符表：平坦模型，初始化内核代码段/数据段、用户代码段/数据段（`kernel/gdt.c`、`boot/gdt_flush.S`）
- [x] I/O 端口读写 + 串口调试：`inb`/`outb` 基础端口操作，COM1 串口初始化与输出，VGA 输出同步到串口便于 QEMU 自动化验证（`kernel/port.c`）

#### 核心层（已完成 5 项，剩余 2 项）

- [x] 中断管理系统：IDT 初始化 + 8259 PIC 重映射 + ISR 汇编（Linux 统一中断框架，3 种宏封装异常/硬件中断/系统调用）+ 异常处理 + 页错误处理（`kernel/idt.c`、`kernel/interrupt.c`、`kernel/isr.S`、`kernel/idt_flush.S`）
- [x] 物理内存布局：解析 Multiboot2 mmap 标签和 basic_meminfo 标签，打印完整物理内存区域（`kernel/memory.c`）
- [x] 物理内存管理 PMM：128KB 位图管理 4GB 物理地址空间，保守初始化策略，页帧分配/释放，内核区域保护（`kernel/pmm.c`）
- [x] 虚拟内存分页 VMM：递归页表映射（PDE[1023] 自映射），页面映射 `vmm_map_page`、物理地址查询 `vmm_get_phys`、页错误处理（`kernel/mmu.c`）
- [x] 高半核分页启动：引导阶段 C 语言设置 PDE[0] identity 映射、PDE[768] 高半核映射、PDE[1023] 递归映射（`kernel/boot_paging.c`）

#### 核心层（剩余 2 项）

- [ ] PIT 定时器驱动：编程 8253/8254 PIT 定时器芯片，产生 IRQ0 周期性时钟中断，是进程调度的基础（依赖：I/O 端口 ✅、中断系统 ✅）
- [ ] 进程控制块（PCB）设计：定义进程/线程数据结构（PID、状态、寄存器上下文、内核栈、页目录等），是进程管理的基础（依赖：PMM ✅、VMM ✅）

#### 进阶层（全部未开始）

- [ ] 上下文切换：实现内核栈切换、寄存器保存/恢复（TSS 或软件切换），支持多任务调度（依赖：PCB、定时器）
- [ ] 用户态切换：利用 GDT 用户段（已定义 ✅）+ TSS 实现 ring0→ring3 切换，建立用户态运行环境
- [ ] 系统调用接口：实现 `int 0x80`（vector 128）的系统调用分发框架，提供用户态→内核态服务调用（IDT 已注册 isr128，DPL=3，但无实际分发逻辑）
- [ ] ELF 可执行文件加载：解析 ELF 格式，将用户程序加载到用户地址空间并跳转执行（依赖：VMM 用户空间映射）

#### 扩展层（全部未开始）

- [ ] VGA 图形模式：从文本模式切换到 VGA 图形模式（如 320x200x256 或 VBE 模式），提供像素级绘图原语
- [ ] 窗口管理：基础窗口系统，窗口创建、移动、重叠、重绘管理
- [ ] 键盘输入：编程键盘控制器（IRQ1），实现 scancode→ASCII 转换，提供输入事件
- [ ] 鼠标输入：编程 PS/2 鼠标控制器（IRQ12），实现光标定位与事件处理

#### 其他改进点

- [ ] 硬件中断未启用：PIC 初始化后屏蔽了所有中断（0xFF），尚未解除特定 IRQ 的屏蔽（如 IRQ0 定时器、IRQ1 键盘）
- [ ] 中断处理函数注册框架闲置：`interrupt_handlers[]` 数组已定义，但未注册任何实际硬件中断处理函数
- [ ] 堆内存分配器：当前仅有页级分配器（PMM），缺少 kmalloc/kfree 等小内存分配器，后续模块开发会需要

### 最新进度请关注公众号

![sfd](docs/公众号.jpg)
