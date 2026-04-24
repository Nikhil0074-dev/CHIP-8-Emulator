#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

/* ── CHIP-8 Constants ─────────────────────────────────────────────────── */
#define MEMORY_SIZE      4096
#define NUM_REGISTERS    16
#define STACK_SIZE       16
#define NUM_KEYS         16
#define DISPLAY_WIDTH    64
#define DISPLAY_HEIGHT   32
#define FONTSET_SIZE     80
#define PROGRAM_START    0x200
#define FONTSET_START    0x050

/* ── CHIP-8 State ─────────────────────────────────────────────────────── */
typedef struct {
    uint8_t  memory[MEMORY_SIZE];           /* 4KB RAM                    */
    uint8_t  V[NUM_REGISTERS];              /* V0–VF general registers    */
    uint16_t I;                             /* Index register             */
    uint16_t PC;                            /* Program counter            */
    uint16_t stack[STACK_SIZE];             /* Call stack                 */
    uint8_t  SP;                            /* Stack pointer              */
    uint8_t  delay_timer;                   /* Decrements at 60 Hz        */
    uint8_t  sound_timer;                   /* Decrements at 60 Hz        */
    uint8_t  display[DISPLAY_WIDTH * DISPLAY_HEIGHT]; /* Pixel buffer     */
    uint8_t  keypad[NUM_KEYS];              /* Key state (1=pressed)      */
    bool     draw_flag;                     /* Screen needs redraw        */
} Chip8;

/* ── Public API ───────────────────────────────────────────────────────── */
void    chip8_init(Chip8 *c8);
bool    chip8_load_rom(Chip8 *c8, const char *path);
void    chip8_cycle(Chip8 *c8);
void    chip8_update_timers(Chip8 *c8);

#endif /* CHIP8_H */
