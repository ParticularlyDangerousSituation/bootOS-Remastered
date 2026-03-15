Install Dependancies:

    NASM - x86 Assembly Compiler

    GCC - C Compiler (with multilib support for 32-bit)

    GNU Binutils - Linker and binary management

    QEMU - i386 System Emulator


After that, run this in your terminal:
cd bootOS

    nasm -f elf32 kernel/entry.asm -o kernel_entry.o && \
    gcc -m32 -ffreestanding -fno-pic -c kernel/kernel.c -o kernel.o && \
    ld -m elf_i386 -T linker.ld kernel_entry.o kernel.o -o kernel.bin && \
    nasm -f bin boot/boot.asm -o boot.bin && \
    cat boot.bin kernel.bin > os-image.bin && \
    truncate -s 20k os-image.bin && \
    qemu-system-i386 -drive format=raw,file=os-image.bin


Thank you for using this kernel.
