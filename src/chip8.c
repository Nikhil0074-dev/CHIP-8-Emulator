/*
 * chip8.c  –  CHIP-8 interpreter: init, ROM loader, CPU cycle, timers
 *
 * Instruction set reference:
 *   https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chip8.h"

/* ── Built-in 4×5 font sprites (0–F) ─────────────────────────────────── */
static const uint8_t FONTSET[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

/* ── Initialise interpreter state ─────────────────────────────────────── */
void chip8_init(Chip8 *c8)
{
    srand((unsigned)time(NULL));
    memset(c8, 0, sizeof(*c8));
    c8->PC = PROGRAM_START;
    /* Load font into reserved memory area */
    memcpy(&c8->memory[FONTSET_START], FONTSET, FONTSET_SIZE);
}

/* ── Load ROM file into memory starting at 0x200 ─────────────────────── */
bool chip8_load_rom(Chip8 *c8, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[CHIP-8] Cannot open ROM: %s\n", path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size > (long)(MEMORY_SIZE - PROGRAM_START)) {
        fprintf(stderr, "[CHIP-8] ROM too large (%ld bytes)\n", size);
        fclose(f);
        return false;
    }

    size_t read = fread(&c8->memory[PROGRAM_START], 1, (size_t)size, f);
    fclose(f);

    if ((long)read != size) {
        fprintf(stderr, "[CHIP-8] ROM read error\n");
        return false;
    }

    printf("[CHIP-8] Loaded ROM: %s (%ld bytes)\n", path, size);
    return true;
}

/* ── Decrement timers (call at 60 Hz) ────────────────────────────────── */
void chip8_update_timers(Chip8 *c8)
{
    if (c8->delay_timer > 0) c8->delay_timer--;
    if (c8->sound_timer > 0) {
        if (c8->sound_timer == 1) printf("\a"); /* Terminal beep */
        c8->sound_timer--;
    }
}

/* ═══════════════════════════════════════════════════════════════════════
 *  CPU CYCLE  –  Fetch → Decode → Execute
 * ═══════════════════════════════════════════════════════════════════════ */
void chip8_cycle(Chip8 *c8)
{
    /* ── FETCH: read big-endian 16-bit opcode ─────────────────────────── */
    uint16_t opcode = (uint16_t)((c8->memory[c8->PC] << 8)
                               | c8->memory[c8->PC + 1]);
    c8->PC += 2;

    /* ── DECODE nibbles ───────────────────────────────────────────────── */
    uint8_t  nibble = (opcode & 0xF000) >> 12;
    uint8_t  x      = (opcode & 0x0F00) >> 8;   /* Vx register index    */
    uint8_t  y      = (opcode & 0x00F0) >> 4;   /* Vy register index    */
    uint8_t  n      =  opcode & 0x000F;          /* 4-bit constant       */
    uint8_t  kk     =  opcode & 0x00FF;          /* 8-bit constant       */
    uint16_t nnn    =  opcode & 0x0FFF;          /* 12-bit address       */

    /* ── EXECUTE ─────────────────────────────────────────────────────── */
    switch (nibble) {

    case 0x0:
        if (opcode == 0x00E0) {
            /* 00E0 – CLS: clear display */
            memset(c8->display, 0, sizeof(c8->display));
            c8->draw_flag = true;
        } else if (opcode == 0x00EE) {
            /* 00EE – RET: return from subroutine */
            c8->PC = c8->stack[--c8->SP];
        }
        break;

    case 0x1:
        /* 1NNN – JP addr */
        c8->PC = nnn;
        break;

    case 0x2:
        /* 2NNN – CALL addr */
        c8->stack[c8->SP++] = c8->PC;
        c8->PC = nnn;
        break;

    case 0x3:
        /* 3XKK – SE Vx, byte: skip if Vx == kk */
        if (c8->V[x] == kk) c8->PC += 2;
        break;

    case 0x4:
        /* 4XKK – SNE Vx, byte: skip if Vx != kk */
        if (c8->V[x] != kk) c8->PC += 2;
        break;

    case 0x5:
        /* 5XY0 – SE Vx, Vy: skip if Vx == Vy */
        if (c8->V[x] == c8->V[y]) c8->PC += 2;
        break;

    case 0x6:
        /* 6XKK – LD Vx, byte */
        c8->V[x] = kk;
        break;

    case 0x7:
        /* 7XKK – ADD Vx, byte (no carry) */
        c8->V[x] += kk;
        break;

    case 0x8:
        switch (n) {
        case 0x0: c8->V[x]  = c8->V[y];  break;  /* 8XY0 LD  */
        case 0x1: c8->V[x] |= c8->V[y];  break;  /* 8XY1 OR  */
        case 0x2: c8->V[x] &= c8->V[y];  break;  /* 8XY2 AND */
        case 0x3: c8->V[x] ^= c8->V[y];  break;  /* 8XY3 XOR */
        case 0x4: {                                /* 8XY4 ADD with carry */
            uint16_t sum = c8->V[x] + c8->V[y];
            c8->V[0xF] = (sum > 0xFF) ? 1 : 0;
            c8->V[x]   = (uint8_t)sum;
            break; }
        case 0x5:                                  /* 8XY5 SUB */
            c8->V[0xF] = (c8->V[x] > c8->V[y]) ? 1 : 0;
            c8->V[x]  -= c8->V[y];
            break;
        case 0x6:                                  /* 8XY6 SHR */
            c8->V[0xF] = c8->V[x] & 0x1;
            c8->V[x] >>= 1;
            break;
        case 0x7:                                  /* 8XY7 SUBN */
            c8->V[0xF] = (c8->V[y] > c8->V[x]) ? 1 : 0;
            c8->V[x]   = c8->V[y] - c8->V[x];
            break;
        case 0xE:                                  /* 8XYE SHL */
            c8->V[0xF] = (c8->V[x] >> 7) & 0x1;
            c8->V[x] <<= 1;
            break;
        }
        break;

    case 0x9:
        /* 9XY0 – SNE Vx, Vy: skip if Vx != Vy */
        if (c8->V[x] != c8->V[y]) c8->PC += 2;
        break;

    case 0xA:
        /* ANNN – LD I, addr */
        c8->I = nnn;
        break;

    case 0xB:
        /* BNNN – JP V0, addr */
        c8->PC = nnn + c8->V[0];
        break;

    case 0xC:
        /* CXKK – RND Vx, byte */
        c8->V[x] = (uint8_t)(rand() & 0xFF) & kk;
        break;

    case 0xD: {
        /* DXYN – DRW Vx, Vy, nibble: draw N-byte sprite at (Vx,Vy) */
        uint8_t xPos = c8->V[x] % DISPLAY_WIDTH;
        uint8_t yPos = c8->V[y] % DISPLAY_HEIGHT;
        c8->V[0xF]   = 0;

        for (int row = 0; row < n; row++) {
            uint8_t sprite_byte = c8->memory[c8->I + row];
            for (int col = 0; col < 8; col++) {
                if (sprite_byte & (0x80 >> col)) {
                    int px = (xPos + col) % DISPLAY_WIDTH;
                    int py = (yPos + row) % DISPLAY_HEIGHT;
                    int idx = py * DISPLAY_WIDTH + px;
                    if (c8->display[idx]) c8->V[0xF] = 1; /* collision */
                    c8->display[idx] ^= 1;
                }
            }
        }
        c8->draw_flag = true;
        break; }

    case 0xE:
        if (kk == 0x9E) {
            /* EX9E – SKP Vx: skip if key Vx pressed */
            if (c8->keypad[c8->V[x]]) c8->PC += 2;
        } else if (kk == 0xA1) {
            /* EXA1 – SKNP Vx: skip if key Vx NOT pressed */
            if (!c8->keypad[c8->V[x]]) c8->PC += 2;
        }
        break;

    case 0xF:
        switch (kk) {
        case 0x07: c8->V[x] = c8->delay_timer;  break; /* FX07 LD Vx, DT  */
        case 0x15: c8->delay_timer = c8->V[x];  break; /* FX15 LD DT, Vx  */
        case 0x18: c8->sound_timer = c8->V[x];  break; /* FX18 LD ST, Vx  */
        case 0x1E: c8->I += c8->V[x];           break; /* FX1E ADD I, Vx  */

        case 0x0A: {
            /* FX0A – LD Vx, K: wait for a key press */
            bool key_found = false;
            for (int i = 0; i < NUM_KEYS; i++) {
                if (c8->keypad[i]) { c8->V[x] = (uint8_t)i; key_found = true; break; }
            }
            if (!key_found) c8->PC -= 2; /* Repeat until key pressed    */
            break; }

        case 0x29:
            /* FX29 – LD F, Vx: set I = location of sprite for digit Vx */
            c8->I = FONTSET_START + (c8->V[x] * 5);
            break;

        case 0x33: {
            /* FX33 – LD B, Vx: store BCD of Vx in memory I, I+1, I+2 */
            uint8_t val = c8->V[x];
            c8->memory[c8->I + 2] = val % 10; val /= 10;
            c8->memory[c8->I + 1] = val % 10; val /= 10;
            c8->memory[c8->I + 0] = val % 10;
            break; }

        case 0x55:
            /* FX55 – LD [I], Vx: store V0–Vx in memory starting at I */
            for (int i = 0; i <= x; i++) c8->memory[c8->I + i] = c8->V[i];
            break;

        case 0x65:
            /* FX65 – LD Vx, [I]: read V0–Vx from memory starting at I */
            for (int i = 0; i <= x; i++) c8->V[i] = c8->memory[c8->I + i];
            break;
        }
        break;

    default:
        fprintf(stderr, "[CHIP-8] Unknown opcode: 0x%04X at PC=0x%03X\n",
                opcode, c8->PC - 2);
        break;
    }
}
