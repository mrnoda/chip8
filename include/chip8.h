#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <SDL2/SDL.h>

#include "cpu.h"

#define C8_MEM_SIZE             0x1000
#define C8_DISPLAY_WIDTH        64
#define C8_DISPLAY_HEIGHT       32

/* Global declarations. */
extern const uint16_t C8_LOAD_ADDR;
extern const size_t C8_SPRITE_LEN;
extern const char *C8_WINDOW_TITLE;
extern const int C8_FPS;
extern int KEYMAP[16]; 

/* Represents a CHIP-8 system, comprising of a CPU, memory, display and keyboard. */ 
struct chip8
{
    struct c8_cpu *cpu;
    uint8_t memory[C8_MEM_SIZE];
    uint8_t display[C8_DISPLAY_WIDTH * C8_DISPLAY_HEIGHT];
    bool keyboard[16];
    bool alive;
    bool beep;
    bool draw;
};

/*
 * Initialise a chip8 instance. This will reset the processor state and clear any IO/display 
 * data. This should be called prior to any attempt to run or destroy the chip8.
 */
bool c8_init(struct chip8 *c8, struct c8_cpu *cpu);

/* 
 * Load a rom file into the CHIP-8 at a given address. 
 * Return the number of bytes loaded, or -1 in the case of an error . 
 */ 
ssize_t c8_load(char *filename, struct chip8 *c8, uint16_t address);

/*
 * Run a chip8 instance, starting at a given memory address. 
 * This function will synchronously execute instructions from the chip8 program rom. 
 * It shall run until the alive flag on the chip8 instance is set to false or there are no further 
 * instructions to execute (program counter has reached the end of the address space).
 *
 * Return value is 0 if the chip8 terminated succesfully, or -1 if termination was due to an 
 * unexpected event such as a CPU exception or invalid memory read/write.
 *
 * Error messages will be written to STDERR.
 */
int c8_run(struct chip8 *c8, uint16_t start_address);

/* Destroy a chip8 instance, this will free any resource handles held by a running chip8 instance. */
void c8_destroy(struct chip8 *c8);

/* Read a single byte from a given memory location. */
uint8_t c8_mem_read8(struct chip8 *c8, uint16_t addr);

/* Read 2 bytes from a given memory location.. */
uint16_t c8_mem_read16(struct chip8 *c8, uint16_t addr);

/* Write a single byte to a given memory location. */
void c8_mem_write8(struct chip8 *c8, uint16_t addr, uint8_t value);

/* Check if a key is currently pressed. Return true if the key is pressed, false otherwise. */
bool c8_key_pressed(struct chip8 *c8, uint8_t key);

/* Wait for a key to be pressed, and return the key that was pressed. */
uint8_t c8_key_await(struct chip8 *c8);

/* Emit a beep sound from a chip8 machine. */
void c8_beep(struct chip8 *c8);

#endif /* CHIP8_H */
