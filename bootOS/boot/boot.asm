[BITS 16]
[ORG 0x7C00]

KERNEL_OFFSET equ 0x1000

start:
    ; Setup stack and segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [BOOT_DRIVE], dl

    ; Print "Loading..."
    mov si, load_msg
    call print_string_16

    ; Load Kernel from Disk
    mov bx, KERNEL_OFFSET 
    mov dh, 15            ; Number of sectors
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02
    mov al, dh
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02          ; Start at sector 2
    int 0x13
    jc disk_error         ; Jump if Carry Flag is set (error)

    ; Switch to Protected Mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:init_pm

disk_error:
    mov si, error_msg
    call print_string_16
    jmp $

print_string_16:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

[BITS 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call KERNEL_OFFSET    ; Jump to C code
    jmp $

; --- GDT Data ---
gdt_start:
    dq 0x0
gdt_code:
    dw 0xFFFF, 0x0
    db 0x0, 10011010b, 11001111b, 0x0
gdt_data:
    dw 0xFFFF, 0x0
    db 0x0, 10010010b, 11001111b, 0x0
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BOOT_DRIVE db 0
load_msg  db 'Loading Kernel...', 13, 10, 0
error_msg db 'Disk Read Error!', 0

times 510-($-$$) db 0
dw 0xAA55
