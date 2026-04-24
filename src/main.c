/*
 * main.c - Entry point for the CHIP-8 emulator
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "chip8.h"
#include "display.h"

#define CPU_HZ            500
#define CYCLES_PER_TIMER  (CPU_HZ / 60)

#ifdef _WIN32
#  include <windows.h>
#  define SLEEP_MS(ms)  Sleep(ms)
#else
#  include <unistd.h>
#  define SLEEP_MS(ms)  usleep((unsigned int)((ms) * 1000))
#endif

static void print_usage(const char *prog)
{
    printf("CHIP-8 Emulator\n");
    printf("  Usage : %s <rom-file>\n", prog);
    printf("          %s --test <rom-file>   (headless, 1000 cycles)\n", prog);
    printf("  Keys  : 1234 / QWER / ASDF / ZXCV  (CHIP-8 0-F), ESC=quit\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) { print_usage(argv[0]); return 1; }

    bool        headless  = false;
    const char *rom_path  = NULL;

    if (strcmp(argv[1], "--test") == 0) {
        if (argc < 3) { print_usage(argv[0]); return 1; }
        headless = true;
        rom_path = argv[2];
    } else {
        rom_path = argv[1];
    }

    Chip8 c8;
    chip8_init(&c8);
    if (!chip8_load_rom(&c8, rom_path)) return 1;

    Display *disp = display_create();
    if (!disp) return 1;

    printf("[Main] Emulator running. ESC or close window to quit.\n");

    int  cycle_count = 0;
    bool running     = true;
    int  max_cycles  = headless ? 1000 : -1;

    while (running) {
        running = display_poll_events(disp, &c8);
        chip8_cycle(&c8);
        cycle_count++;

        if (cycle_count % CYCLES_PER_TIMER == 0)
            chip8_update_timers(&c8);

        if (c8.draw_flag) {
            display_render(disp, c8.display);
            c8.draw_flag = false;
        }

        if (max_cycles > 0 && cycle_count >= max_cycles) {
            printf("[Main] Headless test done (%d cycles).\n", cycle_count);
            break;
        }

        SLEEP_MS(1000 / CPU_HZ);
    }

    display_destroy(disp);
    printf("[Main] Exited after %d cycles.\n", cycle_count);
    return 0;
}
