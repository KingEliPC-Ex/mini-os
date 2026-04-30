#include <stdint.h>
#include <stddef.h>
#include "libc.h"

/* ===========================
   PORT I/O
   =========================== */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ===========================
   SERIAL DRIVER (COM1)
   =========================== */
void serial_init() {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

int serial_ready() {
    return inb(0x3F8 + 5) & 0x20;
}

void serial_write_char(char c) {
    while (!serial_ready());
    outb(0x3F8, c);
}

/* ===========================
   VGA TEXT MODE
   =========================== */
static uint16_t* const VGA = (uint16_t*)0xB8000;
static int cursor = 0;

void vga_put(char c) {
    if (c == '\n') {
        cursor += 80 - (cursor % 80);
        return;
    }
    VGA[cursor++] = (0x07 << 8) | c;
}

void vga_clear() {
    for (int i = 0; i < 80 * 25; i++)
        VGA[i] = (0x07 << 8) | ' ';
    cursor = 0;
}

/* ===========================
   PRINT (dual output)
   =========================== */
void print(const char* s) {
    while (*s) {
        serial_write_char(*s);
        vga_put(*s);
        s++;
    }
}


int key_available() {
    return inb(0x64) & 1;
}

char get_key() {
    uint8_t sc = inb(0x60);
    if (sc & 0x80) return 0;
    return scancode_to_ascii(sc);
}

/* ===========================
   SHELL
   =========================== */
char input_buffer[128];
int input_len = 0;

void prompt() {
    print("miniOS> ");
}

void clear_screen() {
    vga_clear();
    print("miniOS shell ready.\n");
}

void run_command() {
    input_buffer[input_len] = 0;

    if (!input_len) return;

    if (!__builtin_strcmp(input_buffer, "help")) {
        print("Commands: help, clear, echo, reboot\n");
    }
    else if (!__builtin_strcmp(input_buffer, "clear")) {
        clear_screen();
    }
    else if (!__builtin_strncmp(input_buffer, "echo ", 5)) {
        print(input_buffer + 5);
        print("\n");
    }
    else if (!__builtin_strcmp(input_buffer, "reboot")) {
        print("Rebooting...\n");
        outb(0x64, 0xFE);
    }
    else {
        print("Unknown command.\n");
    }
}

/* ===========================
   KERNEL ENTRY
   =========================== */
void kernel_start() {
    serial_init();
    vga_clear();

    print("Welcome to miniOS!\n");
    prompt();

    while (1) {
        if (key_available()) {
            char c = get_key();
            if (!c) continue;

            serial_write_char(c);
            vga_put(c);

            if (c == '\n') {
                run_command();
                input_len = 0;
                prompt();
            } else {
                input_buffer[input_len++] = c;
            }
        }
    }
}