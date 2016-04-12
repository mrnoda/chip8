#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "chip8.h"

uint8_t c8_fontset[C8_FONTSET_LEN] = 
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void c8_init(struct chip8 *c8, struct c8_cpu *cpu)
{
    printf("CHIP-8 Init\n");
    c8->cpu = cpu;
    cpu_init(c8);
    mem_init(c8);
    display_init(c8);
}

void mem_init(struct chip8 *c8)
{
    memset(c8->memory, 0, sizeof c8->memory);
    for (int i = 0; i < 80; i++)
    {
        c8->memory[i] = c8_fontset[i];
    }
}

void display_init(struct chip8 *c8)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialise: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          C8_DISPLAY_WIDTH, C8_DISPLAY_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        fprintf(stderr, "Window could not be creaed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Surface *screen = SDL_GetWindowSurface(window);
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);
    SDL_Delay(2000);
}

void c8_run(struct chip8 *c8)
{
    printf("CHIP-8 Run\n");
    while (true)
    {
        cpu_print(c8->cpu);
        cpu_step(c8->cpu);
    }
}

void c8_destroy(struct chip8 *c8)
{
    printf("CHIP-8 Destroy\n");
}

uint8_t mem_read8(struct chip8 *c8, uint16_t addr)
{
    assert(addr <= C8_MAX_ADDR);
    return c8->memory[addr];    
}

void mem_write8(struct chip8 *c8, uint16_t addr, uint8_t value)
{
    assert(addr >= C8_LOAD_ADDR);
    assert(addr <= C8_MAX_ADDR);
    c8->memory[addr] = value;
}

bool key_pressed(struct chip8 *c8, uint8_t key)
{
    return c8->keyboard[key];
}

uint8_t await_key(struct chip8 *c8)
{
    printf("Waiting for a keypress...\n");
    volatile int flag = 1;
    while (flag)
    {
        ; // do nothing, TODO: wait on a sdl keyevent?
    }
}
