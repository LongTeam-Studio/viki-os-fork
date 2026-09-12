#!/data/data/com.termux/files/usr/bin/bash
set -e

echo "[1/6] 安装 Termux 侧工具..."
pkg update -y
pkg install -y proot-distro qemu-system-i386 clang lld
termux-wake-lock

echo "[2/6] 安装 Ubuntu 容器..."
if ! proot-distro list | grep -q ubuntu; then
    proot-distro install ubuntu
fi

echo "[3/6] 在 Ubuntu 里装依赖并拼装 GRUB 模块..."
proot-distro login ubuntu --bind ~/:/termux-home -- bash -c '
set -e
apt update
apt install -y --no-install-recommends build-essential autoconf automake \
    bison flex libtool pkg-config gawk wget xz-utils xorriso mtools grub-common

mkdir -p /root/grub-i386 && cd /root/grub-i386
if ! ls grub-pc-bin_*.deb >/dev/null 2>&1; then
    wget http://archive.ubuntu.com/ubuntu/pool/main/g/grub2/grub-pc-bin_2.06-2ubuntu7_amd64.deb
fi
dpkg-deb -x grub-pc-bin_*.deb ./
mkdir -p /usr/lib/grub/i386-pc
cp -a ./usr/lib/grub/i386-pc/. /usr/lib/grub/i386-pc/
echo "i386-pc 模块数: $(ls /usr/lib/grub/i386-pc/ | wc -l)"
'

echo "[4/6] 检查项目并修改 Makefile、boot.S..."
cd ~/viki-os-master/viki-os-master

cp Makefile Makefile.orig 2>/dev/null || true
cat > Makefile <<"EOF"
CC = clang --target=i386-elf
AS = clang --target=i386-elf
LD = ld.lld
CFLAGS = -ffreestanding -fno-stack-protector -fno-pic -nostdlib \
         -Wall -Wextra -I./include -g \
         -march=i386 -mno-sse -mno-mmx -mno-80387 -mno-fp-ret-in-387 -msoft-float
ASFLAGS = -g -I./include
LDFLAGS = -m elf_i386 -T linker.ld

BOOT_OBJ = boot/boot.o boot/gdt_flush.o
KERNEL_BIN = kernel.bin
KERNEL_SRC := $(wildcard kernel/*.c)
KERNEL_OBJ := $(patsubst %.c,%.o,$(KERNEL_SRC))
KERNEL_ASM := $(wildcard kernel/*.S)
KERNEL_ASM_OBJ := $(patsubst %.S,%.o,$(KERNEL_ASM))
OBJS = $(BOOT_OBJ) $(KERNEL_OBJ) $(KERNEL_ASM_OBJ)

all: iso

boot/boot.o: boot/boot.S
$(AS) $(ASFLAGS) -c $< -o $@
boot/gdt_flush.o: boot/gdt_flush.S
$(AS) $(ASFLAGS) -c $< -o $@
kernel/%.o: kernel/%.c
$(CC) $(CFLAGS) -c $< -o $@
kernel/%.o: kernel/%.S
$(AS) $(ASFLAGS) -c $< -o $@

$(KERNEL_BIN): $(OBJS) linker.ld
$(LD) $(LDFLAGS) $(OBJS) -o $@

iso: $(KERNEL_BIN)
cp $(KERNEL_BIN) iso/boot/

clean:
rm -f $(OBJS) $(KERNEL_BIN) viki-os.iso iso/boot/$(KERNEL_BIN)

.PHONY: all iso clean
EOF

grep -q 'call \*%eax' boot/boot.S || \
    sed -i 's/^\( *\)call setup_paging_c/\1movl $setup_paging_c, %eax\n\1call *%eax/' boot/boot.S

echo "[5/6] 编译内核..."
make clean
make

echo "[6/6] 打包 ISO..."
proot-distro login ubuntu --bind ~/:/termux-home -- bash -c '
cd /termux-home/viki-os-master/viki-os-master
grub-mkrescue --directory=/usr/lib/grub/i386-pc -o viki-os.iso iso/
'

echo ""
echo "=== 完成 ==="
echo "运行："
echo "  timeout 15 qemu-system-i386 -cdrom viki-os.iso -m 256 \\"
echo "    -accel tcg,tb-size=32 -serial stdio -display none"
