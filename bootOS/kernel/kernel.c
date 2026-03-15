#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0F
#define MAX_FILES 10

/* --- HARDWARE I/O --- */
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "dN"(port));
}

/* --- CURSOR & RENDERING --- */
int cursor_x = 0;
int cursor_y = 0;

void print_char(char c) {
    char* vidptr = (char*)VIDEO_ADDRESS;
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int offset = (cursor_y * MAX_COLS + cursor_x) * 2;
        vidptr[offset] = c;
        vidptr[offset + 1] = WHITE_ON_BLACK;
        cursor_x++;
    }
    if (cursor_x >= MAX_COLS) {
        cursor_x = 0;
        cursor_y++;
    }
}

void print_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) print_char(str[i]);
}

void clear_screen() {
    char* vidptr = (char*)VIDEO_ADDRESS;
    for (int i = 0; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        vidptr[i] = ' ';
        vidptr[i+1] = WHITE_ON_BLACK;
    }
    cursor_x = 0; cursor_y = 0;
}

/* --- STRING TOOLS --- */
int strcmp(char* s1, char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

void strcpy(char* dest, char* src) {
    int i = 0;
    while ((dest[i] = src[i]) != '\0') i++;
}

/* --- RAM FILE SYSTEM --- */
typedef struct {
    char name[16];
    int active;
} File;

File ramfs[MAX_FILES];
int file_count = 0;

void init_fs() {
    for(int i = 0; i < MAX_FILES; i++) ramfs[i].active = 0;
}

/* --- KEYBOARD --- */
unsigned char get_scancode() {
    while (!(port_byte_in(0x64) & 1));
    return port_byte_in(0x60);
}

char scancode_to_ascii(unsigned char code) {
    char map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    if (code == 0x1C) return '\n';
    if (code < 0x3A) return map[code];
    return 0;
}

/* --- SHELL COMMANDS --- */
void execute_command(char* input) {
    if (strcmp(input, "help") == 0) {
        print_string("Commands: dir, enter [name], version, time, echo [msg], sysmonitor, clear\n");
    } 
    else if (strcmp(input, "version") == 0) {
        print_string("LegacyOS v1.0.0 (32-bit Protected Mode)\n");
    } 
    else if (strcmp(input, "dir") == 0) {
        print_string("Contents of RAMFS:\n");
        for(int i = 0; i < MAX_FILES; i++) {
            if(ramfs[i].active) {
                print_string("- "); print_string(ramfs[i].name); print_string("\n");
            }
        }
    } 
    else if (strcmp(input, "sysmonitor") == 0) {
        char vendor[13];
        unsigned int ebx, ecx, edx;
        __asm__ volatile ("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
        *((int*)vendor) = ebx; *((int*)(vendor + 4)) = edx; *((int*)(vendor + 8)) = ecx; vendor[12] = '\0';
        
        print_string("CPU: "); print_string(vendor);
        print_string("\nMEM: 32MB Allocated (QEMU)\nGPU: VGA Text Mode 0xB8000\n");
    } 
    else if (strcmp(input, "clear") == 0) {
        clear_screen();
    }
    else if (input[0] == 'e' && input[1] == 'n' && input[2] == 't' && input[3] == 'e' && input[4] == 'r') {
        if (file_count < MAX_FILES) {
            strcpy(ramfs[file_count].name, &input[6]);
            ramfs[file_count].active = 1;
            file_count++;
            print_string("File created.\n");
        }
    }
    else {
        print_string("Unknown command.\n");
    }
    print_string("> ");
}

/* --- KERNEL MAIN --- */
void main() {
    clear_screen();
    init_fs();
    print_string("Welcome to LegacyOS. Type 'help' to start.\n> ");

    char buffer[64];
    int i = 0;

    while (1) {
        unsigned char sc = get_scancode();
        if (sc & 0x80) continue; 

        char c = scancode_to_ascii(sc);
        if (c > 0) {
            if (c == '\n') {
                print_char('\n');
                buffer[i] = '\0';
                execute_command(buffer);
                i = 0;
            } else {
                print_char(c);
                buffer[i++] = c;
            }
        }
    }
}
