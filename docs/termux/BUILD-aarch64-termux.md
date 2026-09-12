# aarch64 + Termux 从零搭建 VIKI-OS

## 1. Termux 侧准备

    pkg update
    pkg install proot-distro qemu-system-i386 clang lld
    termux-wake-lock

## 2. 装 Ubuntu（proot）

    proot-distro install ubuntu
    proot-distro login ubuntu

## 3. Ubuntu 侧装依赖

一条一条装，避免 OOM：

    apt update
    apt install -y --no-install-recommends build-essential
    apt install -y --no-install-recommends autoconf automake
    apt install -y --no-install-recommends bison flex libtool pkg-config gawk
    apt install -y --no-install-recommends wget xz-utils
    apt install -y --no-install-recommends xorriso mtools
    apt install -y --no-install-recommends grub-common

## 4. 拼装 i386-pc GRUB 模块

    mkdir -p /root/grub-i386 && cd /root/grub-i386
    wget http://archive.ubuntu.com/ubuntu/pool/main/g/grub2/grub-pc-bin_2.06-2ubuntu7_amd64.deb
    dpkg-deb -x grub-pc-bin_*.deb ./
    mkdir -p /usr/lib/grub/i386-pc
    cp -a ./usr/lib/grub/i386-pc/. /usr/lib/grub/i386-pc/
    ls /usr/lib/grub/i386-pc/ | wc -l    # 应为 303

## 5. 修改 Makefile

    CC = clang --target=i386-elf
    AS = clang --target=i386-elf
    LD = ld.lld

    CFLAGS = -ffreestanding -fno-stack-protector -fno-pic -nostdlib \
             -Wall -Wextra -I./include -g \
             -march=i386 -mno-sse -mno-mmx -mno-80387 -mno-fp-ret-in-387 -msoft-float

    LDFLAGS = -m elf_i386 -T linker.ld

## 6. 修复 call setup_paging_c

    sed -i 's/^\( *\)call setup_paging_c/\1movl $setup_paging_c, %eax\n\1call *%eax/' boot/boot.S
    grep -n -B1 -A1 "call \*%eax" boot/boot.S

## 7. 编译

    make clean
    make

## 8. 打包 ISO

    grub-mkrescue --directory=/usr/lib/grub/i386-pc -o viki-os.iso iso/

## 9. 回 Termux 跑 QEMU

    exit

    timeout 15 qemu-system-i386 -cdrom viki-os.iso -m 256 \
      -accel tcg,tb-size=32 \
      -serial stdio -display none

预期输出：

    Hi, I'm VIKI OS...
    Magic: 0x36d76289, Info Addr: 0x...
    GDT initialized successfully!
    Interrupt subsystem initialized successfully!
    === Physical Memory Layout ===
    ... 6 条内存区域 ...
    Physical memory manager initialized.
    === Virtual Memory (Paging) ===
    ...
    Mapping test PASSED!
    Kernel entered protected mode with paging!
    Running in high-half kernel at 0xC0100000+
    System ready.

## 经验总结

| 坑 | 触发条件 | 修复 |
|---|---|---|
| `as` 不认 `.code32` | aarch64 主机 | `clang --target=i386-elf` 代替 `gcc`/`as` |
| `grub-mkrescue` 缺 i386-pc | aarch64 无 grub-pc-bin | aarch64 grub-common + amd64 grub-pc-bin deb |
| `call setup_paging_c` 崩 | clang 跨 object PC 相对重定位 | 改绝对调用 |
| `CPU Exception 6` | clang 默认生成 SSE | `-mno-sse -mno-mmx -mno-80387 -msoft-float` |
| magic 被踩 | 调试时改 `%al` 覆盖 `%eax` | 调试用 `%cl` 或保存/恢复 `%eax` |
| `-m32` 报错 | aarch64 无 multilib | 用 `--target=i386-elf` 代替 |
