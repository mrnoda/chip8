#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "cpu.h"
#include "chip8.h"

const size_t C8_INS_LEN = 2;

static void decrement_timers(struct chip8 *c8);
static void push(struct c8_cpu *cpu, uint16_t value);
static uint16_t pop(struct c8_cpu *cpu);

void cpu_init(struct c8_cpu *cpu)
{
    // Clear registers
    cpu->pc = 0;
    cpu->i = 0;
    for (int i = 0; i <= 0xF; ++i)
    {
        cpu->v[i] = 0;
    }

    // Clear stack
    cpu->sp = 0;
    for (int i = 0; i <= 0xF; i++)
    {
        cpu->stack[i] = 0;
    }

    // Clear timers
    cpu->timer_delay = 0;
    cpu->timer_sound = 0;

    // Seed the PRNG for op 0xCXNN [RND Vx, byte]
    srand(time(NULL));
}

bool cpu_step(struct chip8 *c8)
{
    struct c8_cpu *cpu = c8->cpu;
    const uint16_t OP  = c8_mem_read16(c8, cpu->pc);
    const uint16_t OP_X   = ((OP & 0x0F00) >> 8);
    const uint16_t OP_Y   = ((OP & 0x00F0) >> 4);
    const uint16_t OP_N   = ((OP & 0x000F) >> 0);
    const uint16_t OP_NN  = ((OP & 0x00FF) >> 0);
    const uint16_t OP_NNN = ((OP & 0x0FFF) >> 0);

    cpu->pc += C8_INS_LEN;

    switch (OP & 0xF000)
    {
        case 0x0:
            // Note, there is a further instruction 0nnn (SYS addr) but this is to be 
            // ignored by modern interpreters
            switch (OP & 0xFF)
            {
                case 0xE0:
                    // 0x00E0: clear the display
                    memset(&c8->display[0], 0, sizeof c8->display);
                    c8->draw = true;
                    break;
                case 0xEE:
                    // 0x00EE: return from subroutine
                    cpu->pc = pop(cpu);
                    break;
                default:
                    // 0x0NNN: call subroutine at nnn
                    push(cpu, cpu->pc);
                    cpu->pc = OP_NNN;
                    break;
            }
            break;
        case 0x1000:
            // 0x1NNN: jump to address nnn
            cpu->pc = OP_NNN;
            break;
        case 0x2000:
            // 0x2NNN: call subroutine at nnn
            push(cpu, cpu->pc);
            cpu->pc = OP_NNN;
            break;
        case 0x3000:
            // 0x3XNN: skip the next instruction if VX equals NN
            cpu->pc += (cpu->v[OP_X] == OP_NN) ? C8_INS_LEN : 0;
            break;
        case 0x4000:
            // 0x4XNN: skip the next instruction if VX doesn't equal NN
            cpu->pc += (cpu->v[OP_X] != OP_NN) ? C8_INS_LEN : 0;
            break;
        case 0x5000:
            // 0x5XY0: skip the next instruction if VX equals VY
            cpu->pc += (cpu->v[OP_X] == cpu->v[OP_Y]) ? C8_INS_LEN : 0;
            break;
        case 0x6000:
            // 0x6XNN: set VX to NN
            cpu->v[OP_X] = OP_NN;
            break;
        case 0x7000:
            // 0x7XNN: add NN to VX, store the result in VX
            cpu->v[OP_X] += OP_NN;
            break;
        case 0x8000:
            switch (OP & 0xF)
            {
                case 0x0:
                    // 0x8XY0: set vx to the value of vy
                    cpu->v[OP_X] = cpu->v[OP_Y];
                    break;
                case 0x1:
                    // 0x8XY1: set vx to vx OR vy
                    cpu->v[OP_X] |= cpu->v[OP_Y];
                    break;
                case 0x2:
                    // 0x8XY2: set vx to vx AND vy
                    cpu->v[OP_X] &= cpu->v[OP_Y];
                    break;
                case 0x3:
                    // 0x8XY3: set vx to vx XOR vy
                    cpu->v[OP_X] ^= cpu->v[OP_Y];
                    break;
                case 0x4:
                {
                    // 0x8XY4: add vy to vx, vf is set to 1 when there is carry, 0 when not
                    uint16_t result16 = (uint16_t)cpu->v[OP_X] + (uint16_t)cpu->v[OP_Y];
                    cpu->v[OP_Y] = (uint8_t)result16;
                    cpu->v[0xF] = result16 > 0xFF ? 1 : 0;
                    break;
                }
                case 0x5:
                {
                    // 0x8XY5: vy is subtracted from vx. vf is set to 0 when there is borrow, 1 when not
                    bool borrow = cpu->v[OP_Y] > cpu->v[OP_X];
                    cpu->v[OP_X] -= cpu->v[OP_Y];
                    cpu->v[0xF] = borrow ? 0 : 1;
                    break;
                }
                case 0x6:
                    // 0x8XY6: shift vx right by one, vf is set to the lsb of vx before the shift
                    cpu->v[0xF] = cpu->v[OP_X] & 1;
                    cpu->v[OP_X] >>= 1;
                    break;
                case 0x7:
                {
                    // 0x8XY7: set vx to vy - vx, vf is set to 0 when there is borrow, 1 when not
                    bool borrow = cpu->v[OP_X] > cpu->v[OP_Y];
                    cpu->v[OP_X] = cpu->v[OP_Y] - cpu->v[OP_X];
                    cpu->v[0xF] = borrow ? 0 : 1;
                    break;
                }
                case 0xE:
                    // 0x8XYE: store vy shifted one bit left in vx, set vf to the msb of vx prior to shift
                    cpu->v[0xF] = cpu->v[OP_X] >> 7;
                    cpu->v[OP_X] = cpu->v[OP_Y] << 1;
                    break;
                default:
                    goto illegal_op;
            }
            break;
        case 0x9000:
            // 0x9XY0: skip the next instruction if vx doesnt equal vy
            cpu->pc += (cpu->v[OP_X] != cpu->v[OP_Y]) ? C8_INS_LEN : 0;
            break;
        case 0xA000:
            // 0xANNN: set I to the address NNN
            cpu->i = OP_NNN;
            break;
        case 0xB000:
            // 0xBNNN: jump to the address NNN plus V0
            cpu->pc = OP_NNN + cpu->v[0];
            break;
        case 0xC000:
            // 0xCXNN: set VX to the result of bitwise AND between NN and rand(0,255)
            cpu->v[OP_X] = (rand() & 0x256) & OP_NN;
            break;
        case 0xD000:
        {
            // 0xDXYN: sprite drawing TODO: review algorithm
            const uint8_t HEIGHT = OP_N;
            const uint8_t X = cpu->v[OP_X];
            const uint8_t Y = cpu->v[OP_Y];

            // Clear the collision flag, it will be set if necessary within the loop
            cpu->v[0xF] = 0;
            c8->draw = true;
            for (int row = 0; row < HEIGHT; row++)
            {
                const uint8_t PIXEL = c8_mem_read8(c8, cpu->i + row);
                for (int column = 0; column < 8; column++)
                {
                    if ((PIXEL & (0x80 >> column)) != 0)
                    {
                        if (c8->display[column+X+(row+Y)*C8_DISPLAY_WIDTH])
                        {
                            cpu->v[0xF] = 1;
                        }
                        c8->display[column+X+(row+Y)*C8_DISPLAY_WIDTH] ^= 1;
                    }
                }

            }
            break;
        }
        case 0xE000:
            switch (OP & 0xFF)
            {
                case 0x9E:
                    // 0xEX9E: skip the next instruction if the key stored in vx is pressed
                    cpu->pc += c8_key_pressed(c8, cpu->v[OP_X]) ? C8_INS_LEN : 0;
                    break;
                case 0xA1: 
                    // 0xEXA1: skip the next instruction if the key stored in vx is not pressed
                    cpu->pc += !c8_key_pressed(c8, cpu->v[OP_X]) ? C8_INS_LEN : 0;
                    break;
                default:
                    goto illegal_op;
            }
            break;
        case 0xF000:
            switch (OP & 0xFF)
            {
                case 0x07:
                    // 0xFX07: set vx to the value of the delay timer
                    cpu->v[OP_X] = cpu->timer_delay;
                    break;
                case 0x0A:
                    // 0xFX0A: a key press is awaited, then stored in vx
                    cpu->v[OP_X] = c8_key_await(c8);
                    break;
                case 0x15:
                    // 0xFX15: set the delay timer to vx
                    cpu->timer_delay = cpu->v[OP_X];
                    break;
                case 0x18:
                    // 0xFX18: set the sound timer to vx
                    cpu->timer_sound = cpu->v[OP_X];
                    break;
                case 0x1E:
                    // 0xFX1E: add vx to i
                    cpu->i += cpu->v[OP_X];
                    break;
                case 0x29:
                    // 0xFX29: set i to the location of the sprite character stored in x
                    cpu->i = cpu->v[OP_X] * C8_SPRITE_LEN;
                    break;
                case 0x33:
                    // 0xFX33: store BCD repreentation of vx in memory locations i, i+1, i+2
                    c8_mem_write8(c8, cpu->i, cpu->v[OP_X] / 100);
                    c8_mem_write8(c8, cpu->i + 1, (cpu->v[OP_X] / 10) % 10);
                    c8_mem_write8(c8, cpu->i + 2, cpu->v[OP_X] % 10);
                    break;
                case 0x55:
                    // 0xFX55: store registers v0 through vx in memory starting at location I
                    for (int i= 0; i <= OP_X; i++)
                    {
                        c8_mem_write8(c8, cpu->i + i, cpu->v[i]);
                    }
                    cpu->i += OP_X + 1; 
                    break;
                case 0x65:
                    // 0xFX65: fill v0 to vx (including vx) with values from memory starting at i
                    for (int i = 0; i <= OP_X; i++)
                    {
                        cpu->v[i] = c8_mem_read8(c8, cpu->i + i);
                    }
                    cpu->i += OP_X + 1;
                    break;
                default:
                    goto illegal_op;
            }
            break;
        default:
            goto illegal_op;
    }

    decrement_timers(c8);
    return true;

illegal_op:
        fprintf(stderr, "Illegal Opcode: %#04x\n", OP); 
        return false;
}


static uint16_t pop(struct c8_cpu *cpu)
{
    assert(cpu-> sp > 0);
    uint16_t result = cpu->stack[cpu->sp - 1];
    cpu->sp--;
    return result;
}

static void push(struct c8_cpu *cpu, uint16_t value)
{
    assert(cpu->sp <= sizeof cpu->stack);
    cpu->stack[cpu->sp] = value;
    cpu->sp++;
}

static void decrement_timers(struct chip8 *c8)
{
    struct c8_cpu *cpu = c8->cpu;
    if (cpu->timer_delay)
    {
        cpu->timer_delay--;
    }
    if (cpu->timer_sound)
    {
        cpu->timer_sound--;
        if (cpu->timer_sound == 0)
        {
            c8->beep = true;
        }
    }
}

