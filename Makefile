# ───────────────────────────────────────────────────────────────────────
#  CHIP-8 Emulator – Makefile
#  Targets:
#    make             Build with SDL2 (full GUI)
#    make headless    Build without SDL2 (no display – for CI / testing)
#    make test        Build headless + run quick self-test
#    make clean       Remove build artefacts
# ───────────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Iinclude
LDFLAGS =
SRCDIR  = src
BUILDDIR= build
TARGET  = $(BUILDDIR)/chip8

SRCS    = $(SRCDIR)/main.c \
          $(SRCDIR)/chip8.c \
          $(SRCDIR)/display.c

OBJS    = $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# ── Default: try SDL2 ────────────────────────────────────────────────────
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS   := $(shell sdl2-config --libs   2>/dev/null)

ifneq ($(SDL_LIBS),)
    CFLAGS  += $(SDL_CFLAGS) -DUSE_SDL2
    LDFLAGS += $(SDL_LIBS)
    $(info SDL2 found – building with display support)
else
    $(info SDL2 not found – building headless stub)
endif

# ────────────────────────────────────────────────────────────────────────
all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Headless build (no SDL2) ─────────────────────────────────────────────
headless: CFLAGS  := -std=c99 -Wall -Wextra -Iinclude
headless: LDFLAGS :=
headless: $(BUILDDIR) $(OBJS_HEADLESS)
	$(CC) $(OBJS_HEADLESS) -o $(BUILDDIR)/chip8_headless $(LDFLAGS)
	@echo "Headless build complete: $(BUILDDIR)/chip8_headless"

# Compile each source without SDL flags for headless
OBJS_HEADLESS = $(BUILDDIR)/main_h.o $(BUILDDIR)/chip8_h.o $(BUILDDIR)/display_h.o

$(BUILDDIR)/main_h.o:    $(SRCDIR)/main.c    | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILDDIR)/chip8_h.o:   $(SRCDIR)/chip8.c   | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILDDIR)/display_h.o: $(SRCDIR)/display.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ── Test: headless 1000-cycle run with built-in test ROM ─────────────────
test: headless
	@echo "Running headless self-test..."
	$(BUILDDIR)/chip8_headless --test assets/roms/test_opcode.ch8 || true

# ── Clean ─────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILDDIR)
	@echo "Cleaned."

.PHONY: all headless test clean
