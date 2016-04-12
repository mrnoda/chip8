#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>

#include "cpu.h"

#define C8_MAX_ADDR         0x1000
#define C8_LOAD_ADDR        0x200
#define C8_FONTSET_LEN      80
#define C8_DISPLAY_WIDTH    64
#define C8_DISPLAY_HEIGHT   32

struct chip8
{
    struct c8_cpu *cpu;
    uint8_t memory[C8_MAX_ADDR];
    bool keyboard[0xF];
    uint8_t display[C8_DISPLAY_WIDTH][C8_DISPLAY_WIDTH];
};

/* System fontset. */
extern uint8_t c8_fontset[C8_FONTSET_LEN];

void c8_init(struct chip8 *c8, struct c8_cpu *cpu);

void c8_run(struct chip8 *c8);

void c8_destroy(struct chip8 *c8);

/* Read a single byte from a given memory location. */
uint8_t mem_read8(struct chip8 *c8, uint16_t addr);

/* Write a single byte to a given memory location. */
void mem_write8(struct chip8 *c8, uint16_t addr, uint8_t value);

/* Check if a key is currently pressed. */
bool key_pressed(struct chip8 *c8, uint8_t key);

/* Wait for a key to be pressed, and return the key pressed. */
uint8_t await_key(struct chip8 *c8);

/* Initialise the system memory to its initial state. */
void mem_init(struct chip8 *c8);

/* Initialise the display. */
void display_init(struct chip8 *c8);

static inline uint16_t mem_read16(struct chip8 *c8, uint16_t addr)
{
    return ((uint16_t)mem_read8(c8, addr) << 8) | mem_read8(c8, addr + 1);
}

#endif
