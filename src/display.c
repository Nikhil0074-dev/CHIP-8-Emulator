/*
 * display.c  –  SDL2 window, renderer, pixel drawing, and keyboard input
 */

#include <stdio.h>
#include <stdlib.h>
#include "display.h"

/* SDL2 is conditionally compiled so the project builds without it too   */
#ifdef USE_SDL2
#include <SDL2/SDL.h>

struct Display {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;
};

/* ── CHIP-8 key ↔ keyboard mapping ───────────────────────────────────── *
 *  CHIP-8 hex keypad:     Keyboard mapping:
 *   1 2 3 C                1 2 3 4
 *   4 5 6 D   ─────────>   Q W E R
 *   7 8 9 E                A S D F
 *   A 0 B F                Z X C V
 * ─────────────────────────────────────────────────────────────────────── */
static const SDL_Keycode KEY_MAP[NUM_KEYS] = {
    SDLK_x,    /* 0 */ SDLK_1,    /* 1 */ SDLK_2,    /* 2 */ SDLK_3,  /* 3 */
    SDLK_q,    /* 4 */ SDLK_w,    /* 5 */ SDLK_e,    /* 6 */ SDLK_a,  /* 7 */
    SDLK_s,    /* 8 */ SDLK_d,    /* 9 */ SDLK_z,    /* A */ SDLK_c,  /* B */
    SDLK_4,    /* C */ SDLK_r,    /* D */ SDLK_f,    /* E */ SDLK_v   /* F */
};

Display *display_create(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "[SDL2] Init failed: %s\n", SDL_GetError());
        return NULL;
    }

    Display *d = calloc(1, sizeof(Display));
    if (!d) return NULL;

    d->window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH  * SCALE,
        DISPLAY_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN);

    if (!d->window) {
        fprintf(stderr, "[SDL2] Window failed: %s\n", SDL_GetError());
        free(d);
        return NULL;
    }

    d->renderer = SDL_CreateRenderer(d->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!d->renderer) {
        fprintf(stderr, "[SDL2] Renderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(d->window);
        free(d);
        return NULL;
    }

    d->texture = SDL_CreateTexture(
        d->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        DISPLAY_WIDTH, DISPLAY_HEIGHT);

    SDL_RenderSetLogicalSize(d->renderer,
        DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE);

    return d;
}

void display_destroy(Display *d)
{
    if (!d) return;
    if (d->texture)  SDL_DestroyTexture(d->texture);
    if (d->renderer) SDL_DestroyRenderer(d->renderer);
    if (d->window)   SDL_DestroyWindow(d->window);
    free(d);
    SDL_Quit();
}

void display_render(Display *d, const uint8_t *pixels)
{
    /* Build RGBA pixel buffer from 1-bit CHIP-8 display */
    static uint32_t buf[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        buf[i] = pixels[i] ? 0xFFFFFFFF : 0x000000FF; /* white / black  */
    }

    SDL_UpdateTexture(d->texture, NULL, buf, DISPLAY_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(d->renderer);
    SDL_RenderCopy(d->renderer, d->texture, NULL, NULL);
    SDL_RenderPresent(d->renderer);
}

bool display_poll_events(Display *d, Chip8 *c8)
{
    (void)d;
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) return false;

        if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
            uint8_t state = (evt.type == SDL_KEYDOWN) ? 1 : 0;

            /* Escape → quit */
            if (evt.key.keysym.sym == SDLK_ESCAPE) return false;

            for (int k = 0; k < NUM_KEYS; k++) {
                if (evt.key.keysym.sym == KEY_MAP[k]) {
                    c8->keypad[k] = state;
                    break;
                }
            }
        }
    }
    return true;
}

#else  /* ── Stub implementation when SDL2 is not available ───────────── */

struct Display { int dummy; };

Display *display_create(void)
{
    Display *d = calloc(1, sizeof(Display));
    printf("[Display] Running in headless/stub mode (SDL2 not available).\n");
    return d;
}
void display_destroy(Display *d) { free(d); }
void display_render(Display *d, const uint8_t *pixels) { (void)d; (void)pixels; }
bool display_poll_events(Display *d, Chip8 *c8)
{
    (void)d; (void)c8;
    return true; /* Never quits in stub mode */
}

#endif /* USE_SDL2 */
