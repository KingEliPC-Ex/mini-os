#!/bin/bash
set -e

mkdir -p build
mkdir -p build/iso/boot/grub

echo "[1] Assembling multiboot header..."
nasm -f elf32 kernel/multiboot_header.asm -o build/multiboot_header.o

echo "[2] Compiling kernel..."
gcc -m32 -ffreestanding -c kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -c kernel/keyboard.c -o build/keyboard.o

echo "[3] Linking kernel..."
ld -m elf_i386 -T link.ld -o build/kernel.elf build/multiboot_header.o build/kernel.o build/keyboard.o

echo "[4] Creating GRUB config..."
cat > build/iso/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "miniOS" {
    multiboot /boot/kernel.elf
}
EOF

echo "[5] Copying kernel..."
cp build/kernel.elf build/iso/boot/kernel.elf

echo "[6] Building ISO..."
grub-mkrescue -o build/miniOS.iso build/iso --xorriso=/usr/bin/xorriso

echo "[7] Running QEMU..."
qemu-system-x86_64 -cdrom build/miniOS.iso -boot d -nographic -serial mon:stdio