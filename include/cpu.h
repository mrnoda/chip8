#ifndef C8_CPU_H
#define C8_CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "chip8.h"

struct chip8;

/* The size, in bytes, of a CHIP-8 CPU instruction. */
extern const size_t C8_INS_LEN;

/*
 * Represents a CHIP-8 processor capable of fetch, decode and execute of 
 * the CHIP-8 instruction set.
 */
struct c8_cpu
{
    /* General-purpose registers. */
    uint8_t v[0x10];

    /* Timer registers. */
    uint8_t timer_delay;
    uint8_t timer_sound;

    /* Flow control registers. */
    uint16_t pc;
    uint8_t sp;
    uint16_t i;

    /* A stack for local variables and call handling. */
    uint16_t stack[0x10];
};

/*
 * Initialise a CPU to its initial state. All registers and stack memory 
 * will be cleared. The state of the CPU after initialisation will be as 
 * follows:
 *
 * PC: 0
 * SP: 0
 * I: 0
 * V0-vF: 0
 * Stack: Empty
 * Delay timer: 0
 * Sound timer: 0
 */
void cpu_init(struct c8_cpu *cpu);

/* 
 * Fetch, decode, and execute a single CHIP-8 instruction. Return true if the instruction was 
 * succesfully executed, false otherwise. Error messages will be written to STDERR.
 */
bool cpu_step(struct chip8 *c8);

#endif /* C8_CPU_H */
