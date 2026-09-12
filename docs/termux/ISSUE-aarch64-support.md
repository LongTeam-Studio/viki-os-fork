# 在 aarch64 主机（Termux/Android）上交叉编译并运行 VIKI-OS

## 环境

- 主机：Android + Termux（aarch64）
- 容器：`proot-distro` Ubuntu
- 工具链：`clang --target=i386-elf` + `ld.lld`
- 目标：i386 保护模式内核，Multiboot2 引导，BIOS 启动

## 问题概述

VIKI-OS 默认按 x86_64 Linux 主机设计（`gcc -m32`、`grub-mkrescue`）。在 aarch64 + Termux 环境下，会遇到三类问题：

1. **工具链缺失**：aarch64 仓库无 `gcc-i686-linux-gnu`，`grub-pc-bin` 也没有 i386-pc 模块
2. **clang 重定位 bug**：跨 `.boot` 段的 `call` 生成 `R_386_PC32`，链接偏移错误
3. **SSE 指令越界**：clang 默认为目标生成 SSE 指令，内核未初始化 FPU/SSE，执行 64 位运算时抛 `#UD`

## 坑 1：`grub-mkrescue` 在 aarch64 上无 i386-pc 模块

**症状**：

    grub-mkrescue: command not found

**根因**：`grub-pc-bin` 只在 amd64 仓库提供。

**修复**：用 aarch64 的 `grub-common` + amd64 的 `grub-pc-bin` 拼装：

    apt install -y --no-install-recommends grub-common xorriso mtools

    mkdir -p /root/grub-i386 && cd /root/grub-i386
    wget http://archive.ubuntu.com/ubuntu/pool/main/g/grub2/grub-pc-bin_2.06-2ubuntu7_amd64.deb
    dpkg-deb -x grub-pc-bin_*.deb ./
    mkdir -p /usr/lib/grub/i386-pc
    cp -a ./usr/lib/grub/i386-pc/. /usr/lib/grub/i386-pc/

    grub-mkrescue --directory=/usr/lib/grub/i386-pc -o viki-os.iso iso/

**原理**：`grub-mkimage` 是主机工具，不产生目标码；i386-pc 模块是纯数据，可跨架构使用。

## 坑 2：`call setup_paging_c` 重定位错误

**症状**：串口只在 `_start` 打出第一个标记，之后无输出。

**链接器警告**：

    ld.lld: warning: boot/boot.o:(.boot+0xf): has non-ABS relocation R_386_PC32 against symbol 'setup_paging_c'

**根因**：`setup_paging_c` 通过 `__attribute__((section(".boot")))` 放进 `.boot` 段。clang 生成 PC 相对 `call`（`R_386_PC32`），跨 object 时链接器算错偏移。

**修复**：

    movl $setup_paging_c, %eax
    call *%eax

## 坑 3：clang 生成 SSE 指令，64 位运算抛 `#UD`

**症状**：遍历 mmap 条目时抛 `CPU Exception 6: Invalid Opcode`。

**根因**：`uint64_t` 运算被编译成 SSE2 指令，内核未初始化 FPU/SSE。

**修复**：Makefile 的 `CFLAGS` 加：

    -march=i386 -mno-sse -mno-mmx -mno-80387 -mno-fp-ret-in-387 -msoft-float

## 建议上游修改

1. `boot/boot.S`：`call setup_paging_c` 改为绝对调用
2. `Makefile`：`CFLAGS` 加 `-mno-sse` 系列
3. `README.md`：加一节「aarch64/Android Termux 构建指引」
