#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "chip8.h"

struct c8_cpu
{
    /* General-purpose registers. */
    uint8_t v[0xF];

    /* Timer registers. */
    uint8_t timer_delay;
    uint8_t timer_sound;

    /* Flow control registers. */
    uint16_t pc;
    uint8_t sp;
    uint16_t i;

    /* The system. */
    struct chip8 *c8;

    /* A stack for local variables and call handling. */
    uint16_t stack[0xF];
};

/* Initialise a cpu instance. */
void cpu_init(struct chip8 *c8);

/* Fetch and execute the next instruction from memory. */
void cpu_step(struct c8_cpu *cpu);

/* Print the CPU registers to stdout. */
void cpu_print(struct c8_cpu *cpu);

/* Pop the topmost value from the stack and update the stack pointer. */
uint16_t pop(struct c8_cpu *cpu);

/* Push an address onto the stack and update the stack pointer. */
void push(struct c8_cpu *cpu, uint16_t value);

#endif
