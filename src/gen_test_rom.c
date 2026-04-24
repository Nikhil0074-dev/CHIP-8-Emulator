/*
 * gen_test_rom.c  –  Generates a small CHIP-8 ROM that exercises the
 *                    most common opcodes so the emulator can be verified
 *                    without needing a real game ROM.
 *
 * Build & run:
 *   gcc gen_test_rom.c -o gen_test_rom && ./gen_test_rom
 *
 * Generates: assets/roms/test_opcode.ch8
 *
 * What the ROM does (headless viewable via register trace):
 *   1. Load values into V0–V3
 *   2. Add, AND, XOR operations
 *   3. Draw a small sprite at (0,0)
 *   4. Loop forever (JP self)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void)
{
    /* Each CHIP-8 instruction is exactly 2 bytes, big-endian */
    uint8_t rom[64];
    int p = 0;

#define OP(w) do { rom[p++] = (uint8_t)((w) >> 8); \
                   rom[p++] = (uint8_t)((w) & 0xFF); } while(0)

    /* Clear screen */
    OP(0x00E0);

    /* V0 = 0x05, V1 = 0x0A */
    OP(0x6005);   /* LD V0, 5  */
    OP(0x610A);   /* LD V1, 10 */

    /* V2 = V0 + V1 (should be 15, VF = 0) */
    OP(0x8014);   /* ADD V0, V1 → V0 = 15 */

    /* V3 = 0xFF, V4 = 0x0F */
    OP(0x63FF);   /* LD V3, 0xFF */
    OP(0x640F);   /* LD V4, 0x0F */

    /* V3 &= V4  (should be 0x0F) */
    OP(0x8342);   /* AND V3, V4  */

    /* V5 ^= V4  */
    OP(0x6500);   /* LD V5, 0    */
    OP(0x6FFF);   /* LD VF, 0xFF */
    OP(0x8F53);   /* XOR VF, V5  */

    /* Set I to font for digit '5' and draw at (10,5) */
    OP(0x6A0A);   /* LD VA, 10 (x) */
    OP(0x6B05);   /* LD VB, 5  (y) */
    OP(0xF529);   /* LD F, V5: I = sprite for 5 */
    OP(0xDAB5);   /* DRW VA, VB, 5 */

    /* Draw digit '0' at (20, 5) */
    OP(0x6C14);   /* LD VC, 20 */
    OP(0xF029);   /* LD F, V0: I = sprite for 0 (V0=15=0xF → shows 'F') */
    OP(0xDCB5);   /* DRW VC, VB, 5 */

    /* Infinite loop (JP to this address) */
    uint16_t loop_addr = 0x200 + p;
    OP(0x1000 | loop_addr);

#undef OP

    /* Write file */
    FILE *f = fopen("assets/roms/test_opcode.ch8", "wb");
    if (!f) { perror("Cannot write ROM"); return 1; }
    fwrite(rom, 1, (size_t)p, f);
    fclose(f);

    printf("Generated test_opcode.ch8 (%d bytes)\n", p);
    return 0;
}
