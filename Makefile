all: build/os.bin

build/boot.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o build/boot.bin

build/kernel.o: kernel/kernel.c
	gcc -m32 -ffreestanding -c kernel/kernel.c -o build/kernel.o

build/keyboard.o: kernel/keyboard.c
	gcc -m32 -ffreestanding -c kernel/keyboard.c -o build/keyboard.o

build/kernel.elf: build/kernel.o build/keyboard.o
	ld -m elf_i386 -T link.ld -o build/kernel.elf build/kernel.o build/keyboard.o

build/os.bin: build/boot.bin build/kernel.elf
	cat build/boot.bin build/kernel.elf > build/os.bin

run: build/os.bin
	qemu-system-x86_64 -nographic -serial mon:stdio -kernel build/os.bin

clean:
	rm -rf build/*