# CHIP-8 Emulator

A complete CHIP-8 virtual machine written in **C (C99)** with optional **SDL2** rendering.

---

## Project Structure

```
chip8-emulator/
├── src/
│   ├── main.c           # Entry point, main loop
│   ├── chip8.c          # CPU, memory, all 35 opcodes
│   ├── display.c        # SDL2 rendering + keyboard input
│   └── gen_test_rom.c   # Utility: generates a test ROM
├── include/
│   ├── chip8.h          # Chip8 struct + public API
│   └── display.h        # Display struct + public API
├── assets/roms/         # Place .ch8 ROM files here
├── build/               # Compiled output (auto-created)
├── Makefile
└── README.md
```

---

## Building

### With SDL2 (full GUI window)

```bash
# Ubuntu / Debian
sudo apt-get install libsdl2-dev

# macOS
brew install sdl2

# Build
make
```

### Without SDL2 (headless / stub mode)

```bash
make headless
```

### Generate and run the built-in test ROM

```bash
# Build the ROM generator and create the test ROM
gcc src/gen_test_rom.c -o build/gen_test_rom
./build/gen_test_rom

# Run headless test (1000 cycles)
make test
```

---

## Running

```bash
# Full GUI (requires SDL2 + a ROM file)
./build/chip8 assets/roms/yourGame.ch8

# Headless mode
./build/chip8 --test assets/roms/yourGame.ch8
```

Where to find ROMs: many public-domain CHIP-8 ROMs are freely available online
(search "CHIP-8 ROM pack").  A ROM file has the `.ch8` extension.

---

## Key Mapping

```
CHIP-8 Keypad        Keyboard
┌─────────────┐      ┌─────────────┐
│ 1  2  3  C  │      │ 1  2  3  4  │
│ 4  5  6  D  │  →   │ Q  W  E  R  │
│ 7  8  9  E  │      │ A  S  D  F  │
│ A  0  B  F  │      │ Z  X  C  V  │
└─────────────┘      └─────────────┘
ESC = Quit
```

---

## Implemented Opcodes

| Opcode | Mnemonic | Description |
|--------|----------|-------------|
| 00E0 | CLS | Clear display |
| 00EE | RET | Return from subroutine |
| 1NNN | JP | Jump to address |
| 2NNN | CALL | Call subroutine |
| 3XKK | SE | Skip if Vx == kk |
| 4XKK | SNE | Skip if Vx != kk |
| 5XY0 | SE | Skip if Vx == Vy |
| 6XKK | LD | Load byte into Vx |
| 7XKK | ADD | Add byte to Vx |
| 8XY0–E | ALU | OR, AND, XOR, ADD, SUB, SHR, SUBN, SHL |
| 9XY0 | SNE | Skip if Vx != Vy |
| ANNN | LD I | Set I = NNN |
| BNNN | JP V0 | Jump to NNN + V0 |
| CXKK | RND | Random byte AND kk |
| DXYN | DRW | Draw sprite (with collision detection) |
| EX9E | SKP | Skip if key Vx pressed |
| EXA1 | SKNP | Skip if key Vx not pressed |
| FX07/15/18 | Timers | Delay/sound timer ops |
| FX0A | LD K | Wait for keypress |
| FX1E | ADD I | I += Vx |
| FX29 | LD F | I = font sprite for Vx |
| FX33 | LD B | BCD of Vx into memory |
| FX55/65 | LD | Store/load registers |

---

## Architecture

```
User Input (Keyboard)
        │
        ▼
  display_poll_events()
        │
        ▼
   chip8_cycle()          ← Fetch → Decode → Execute
        │
   ┌────┴──────────────────────────────────┐
   │ Memory[4KB]  Registers[V0-VF]  Stack  │
   │ Index(I)     PC    Timers             │
   └────┬──────────────────────────────────┘
        │
        ▼
  display_render()        ← 64×32 pixel grid scaled ×10
        │
        ▼
   SDL2 Window
```

---

## Technologies

| Component | Technology |
|-----------|-----------|
| Language  | C (C99)   |
| Graphics  | SDL2      |
| Build     | GNU Make  |
| Platform  | Linux / macOS / Windows |

---
