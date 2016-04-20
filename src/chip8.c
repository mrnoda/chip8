#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "cpu.h"

/* Global Definitions. */
const uint16_t C8_MAX_ADDR = 0x1000;
const uint16_t C8_LOAD_ADDR = 0x200;
const size_t C8_SPRITE_LEN = 5;
const char *C8_WINDOW_TITLE = "CHIP-8";
const int C8_FPS = 300;

/* Static storage. */
SDL_Window *window = NULL;
SDL_Surface *back_buffer = NULL;

/* Print the status of the C8 machine. */
static void c8_print(struct chip8 *c8);

/* Process system flags such as beep/display and trigger system behaviours. */
static void c8_process_flags(struct chip8 *c8);

/* Process SDL input. */
static void c8_process_input(struct chip8 *c8);

/* SDL management. */
static bool c8_display_init(void);
static void c8_display_destroy(void);
static void c8_display_update(struct chip8 *c8);
static void c8_display_draw(void);
static void c8_keyboard_init(struct chip8 *c8);
static void c8_handle_key_event(SDL_KeyboardEvent *key, struct chip8 *c8);

const uint8_t C8_FONTSET[] = 
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

/*
 * An ordered mapping of SDL keyboard symbols representing the configured input keys for 
 * manipulating the CHIP-8 keyboard. The index of the symbol represents the CHIP8 key that 
 * will be considered the source of any event raised. The following diagram illustrates the 
 * mapping of each key on a standard keyboard to the CHIP-8 keyboard:
 *
 * Keypad                   Keyboard
 * +-+-+-+-+                +-+-+-+-+
 * |1|2|3|C|                |1|2|3|C|
 * +-+-+-+-+                +-+-+-+-+
 * |4|5|6|D|                |Q|W|E|R|
 * +-+-+-+-+      =>        +-+-+-+-+
 * |7|8|9|E|                |A|S|D|F|
 * +-+-+-+-+                +-+-+-+-+
 * |A|0|B|F|                |Z|X|C|V|
 * +-+-+-+-+                +-+-+-+-+
 */
int KEYMAP[16];

bool c8_init(struct chip8 *c8, struct c8_cpu *cpu)
{
    /* CPU init. */
    c8->cpu = cpu;
    cpu_init(cpu);

    /* Clear memory, then inject the chip8 fontset at the start address. */
    memset(c8->memory, 0, sizeof c8->memory);
    for (int i = 0; i < sizeof C8_FONTSET; i++)
    {
        c8->memory[i] = C8_FONTSET[i];
    }

    /* Display init. */
    if (!c8_display_init())
    {
        fprintf(stderr, "Failed to initialise display\n");
        return false;
    }

    /* Keyboard Init. */
    c8_keyboard_init(c8);

    /* Flush the SDL input event queue to prevent KEYDOWN on startup. */
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_KEYDOWN);
    SDL_FlushEvent(SDL_KEYUP);

    /* Flags init. */
    c8->draw = false;
    c8->beep = false;

    return true;
}

void c8_keyboard_init(struct chip8 *c8)
{
    KEYMAP[0]   = SDLK_x;
    KEYMAP[1]   = SDLK_1;
    KEYMAP[2]   = SDLK_2;
    KEYMAP[3]   = SDLK_3;
    KEYMAP[4]   = SDLK_q;
    KEYMAP[5]   = SDLK_w;
    KEYMAP[6]   = SDLK_e;
    KEYMAP[7]   = SDLK_a;
    KEYMAP[8]   = SDLK_s;
    KEYMAP[9]   = SDLK_d;
    KEYMAP[0xA] = SDLK_z;
    KEYMAP[0xB] = SDLK_c;
    KEYMAP[0xC] = SDLK_4;
    KEYMAP[0xD] = SDLK_r;
    KEYMAP[0xE] = SDLK_f;
    KEYMAP[0xF] = SDLK_v;
}

ssize_t c8_load(char *filename, struct chip8 *c8, uint16_t address)
{
    static const ssize_t ERROR_VALUE = -1;
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        perror("Failed to open file");
        return ERROR_VALUE;
    }

    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    rewind(f);
    size_t bytes_read = fread(&c8->memory[address],1, size, f);
    if (bytes_read != size)
    {
        fprintf(stderr, "Failed to read file contents\n");
        fprintf(stderr, "File size: %zu, bytes read: %zu\n", size, bytes_read);
        fclose(f);
        return ERROR_VALUE;
    }

    fclose(f);
    return bytes_read;
}

int c8_run(struct chip8 *c8, uint16_t start_address)
{
    printf("CHIP-8 Run\n");
    c8->cpu->pc = start_address;
    c8->alive = true;
    Uint32 start;
    while (c8->alive)
    {
        start = SDL_GetTicks();
        c8_print(c8);
        if (!cpu_step(c8))
        {
            fprintf(stderr, "CPU exception occurred\n");
            return -1;
        }
        c8_process_input(c8);
        c8_process_flags(c8);
        c8_display_draw();

        if (1000/C8_FPS > SDL_GetTicks() - start)
        {
            SDL_Delay(1000/C8_FPS-(SDL_GetTicks()-start));
        }
    }
    c8_destroy(c8);
    return 0;
}

void c8_destroy(struct chip8 *c8)
{
    printf("CHIP-8 Destroy\n");
    c8_display_destroy();
}

uint8_t c8_mem_read8(struct chip8 *c8, uint16_t addr)
{
    /* Allow access to all CHIP8 memory from the start to the end of the address space. */
    assert(addr <= C8_MAX_ADDR);
    return c8->memory[addr];    
}

uint16_t c8_mem_read16(struct chip8 *c8, uint16_t addr)
{
    return ((uint16_t)c8_mem_read8(c8, addr) << 8) | c8_mem_read8(c8, addr + 1);
}

void c8_mem_write8(struct chip8 *c8, uint16_t addr, uint8_t value)
{
    /* It is only valid to write memory within the bounds of the C8 program ROM. */
    assert(addr >= C8_LOAD_ADDR);
    assert(addr <= C8_MAX_ADDR);
    c8->memory[addr] = value;
}
bool c8_key_pressed(struct chip8 *c8, uint8_t key)
{
    return c8->keyboard[key] > 0 ? true : false;
}

uint8_t c8_key_await(struct chip8 *c8)
{
    while (true)
    {
        for (int i = 0; i < 16; i++)
        {
            if (c8_key_pressed(c8, i))
            {
                return i;
            }
        }        
        c8_process_input(c8);
    }
}

static void c8_print(struct chip8 *c8)
{
    printf("[PC:%#04x, SP:%#04x, I:%#04x, OP:%#04x]\n", 
            c8->cpu->pc, c8->cpu->sp, c8->cpu->i, c8_mem_read16(c8, c8->cpu->pc));
    for (int i = 0; i <= 0xF; i++)
    {
        printf("\tv%x:%04x    s%x:%#04x\n", i, c8->cpu->v[i], i, c8->cpu->stack[i]);
    }
    printf("--------------------------------------\n");
}

static bool c8_display_init(void)
{
    static const int DISPLAY_WIDTH = 640;
    static const int DISPLAY_HEIGHT = 480;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(C8_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
         DISPLAY_WIDTH, DISPLAY_HEIGHT, SDL_WINDOW_SHOWN | 0);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to create SDL Window: %s\n", SDL_GetError());
        return false;
    }

    back_buffer = SDL_CreateRGBSurface(0, C8_DISPLAY_WIDTH, C8_DISPLAY_HEIGHT, 32, 0, 0, 0, 0);
    if (back_buffer == NULL)
    {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

static void c8_display_destroy(void)
{
    printf("Destroying SDL context\n");
    SDL_FreeSurface(back_buffer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void c8_process_flags(struct chip8 *c8)
{
    if (c8->draw)
    {
        c8->draw = false;
        c8_display_update(c8);
    }

    if (c8->beep)
    {
        c8->beep = false;
    }
}

static void c8_handle_key_event(SDL_KeyboardEvent *key_event, struct chip8 *c8)
{
    SDL_Keysym key = key_event->keysym;
    if (key.sym == SDLK_ESCAPE)
    {
        c8->alive = false;
        return;
    }

    for (int index = 0; index < 16; index++)
    {
        if (key.sym == KEYMAP[index])
        {
            c8->keyboard[index] = key_event->type == SDL_KEYDOWN ? true : false;
        }
    }
}

static void c8_process_input(struct chip8 *c8)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                c8_handle_key_event(&event.key, c8);
                break;
            case SDL_QUIT:
                c8->alive = false;
                break;
            default:
                break;
        }
    }
}

static void c8_display_update(struct chip8 *c8)
{
    SDL_FillRect(back_buffer, NULL, SDL_MapRGB(back_buffer->format, 0, 0, 0));
    SDL_Rect rect = { .w = 1, .h = 1 };
    for (int x = 0; x < C8_DISPLAY_WIDTH; x++)
    {
        for (int y = 0; y < C8_DISPLAY_HEIGHT; y++)
        {
            rect.y = y;
            rect.x = x;
            if (c8->display[x + y * C8_DISPLAY_WIDTH] > 0)
            {
                SDL_FillRect(back_buffer, &rect, SDL_MapRGB(back_buffer->format, 0xFF, 0xFF, 0xFF));
            }
        }
    } 

    SDL_BlitScaled(back_buffer, NULL, SDL_GetWindowSurface(window), NULL);
}

static void c8_display_draw(void)
{
    SDL_UpdateWindowSurface(window);
}
