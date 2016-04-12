#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cpu.h"
#include "chip8.h"

void cpu_init(struct chip8 *c8)
{
    c8->cpu->c8 = c8;
    c8->cpu->pc = C8_LOAD_ADDR;
    c8->cpu->sp = 0;
    c8->cpu->i = 0;
    srand(time(NULL));
}

void cpu_step(struct c8_cpu *cpu)
{
    uint16_t op = mem_read16(cpu->c8, cpu->pc);
    uint16_t x = ((op & 0x0F00) >> 8);
    uint16_t y = ((op & 0xF0) >> 4);
    uint16_t nn = (op & 0xFF);
    uint16_t nnn = (op & 0xFFF);

    cpu->pc += 2;

    switch (op & 0xF000)
    {
        case 0x0:
        {
            // Note, there is a further instruction 0nnn (SYS addr) but this is to be 
            // ignored by modern interpreters
            switch (op & 0xFF)
            {
                case 0xE0:
                {
                    // 0x00E0: clear the display
                    break;
                }
                case 0xEE:
                {
                    // 0x00EE: return from subroutine
                    cpu->pc = pop(cpu);
                    break;
                }
                default:
                {
                    goto illegal_op;
                    break;
                }
            }
            break;
        }
        case 0x1000:
        {
            // 0x1NNN: jump to address nnn
            cpu->pc = nnn;
            break;
        }
        case 0x2000:
        {
            // 0x2NNN: call subroutine at nnn
            push(cpu, cpu->pc);
            cpu->pc = nnn;
            break;
        }
        case 0x3000:
        {
            // 0x3XNN: skip the next instruction if VX equals NN
            if (cpu->v[x] == nn)
            {
                cpu->pc += 2;
            }
            break;
        }
        case 0x4000:
        {
            // 0x4XNN: skip the next instruction if VX doesn't equal NN
            if (cpu->v[x] != nn)
            {
                cpu->pc += 2;
            }
            break;
        }
        case 0x5000:
        {
            // 0x5XY0: skip the next instruction if VX equals VY
            if (cpu->v[x] == cpu->v[y])
            {
                cpu->pc +=2;
            }
            break;
        }
        case 0x6000:
        {
            // 0x6XNN: set VX to NN
            cpu->v[x] = nn;
            break;
        }
        case 0x7000:
        {
            // 0x7XNN: add NN to VX
            cpu->v[x] += nn;
            break;
        }
        case 0x8000:
        {
            switch (op & 0xF)
            {
                case 0x0:
                {
                    // 0x8XY0: set vx to the value of vy
                    cpu->v[x] = cpu->v[y];
                    break;
                }
                case 0x1:
                {
                    // 0x8XY1: set vx to vx OR vy
                    cpu->v[x] |= cpu->v[y];
                    break;
                }
                case 0x2:
                {
                    // 0x8XY2: set vx to vx AND vy
                    cpu->v[x] |= cpu->v[y];
                    break;
                }
                case 0x3:
                {
                    // 0x8XY3: set vx to vx XOR vy
                    cpu->v[x] ^= cpu->v[y];
                    break;
                }
                case 0x4:
                {
                    // 0x8XY4: add vy to vx, vf is set to 1 when there is carry, 0 when not
                    uint16_t result16 = (uint16_t)cpu->v[x] + (uint16_t)cpu->v[y];
                    cpu->v[y] = (uint8_t)result16;
                    cpu->v[0xF] = result16 > 0xFF ? 1 : 0;
                    break;
                }
                case 0x5:
                {
                    // 0x8XY5: vy is subtracted from vx. vf is set to 0 when there is borrow, 1 when not
                    int16_t result16 = (int)cpu->v[x] - (int)cpu->v[y];
                    cpu->v[x] = (uint8_t)result16;
                    cpu->v[0xF] = result16 >= 0 ? 1 : 0;
                    break;
                }
                case 0x6:
                {
                    // 0x8XY6: shift vx right by one, vf is set to the lsb of vx before the shift
                    uint8_t vx = cpu->v[x];
                    cpu->v[0xF] = vx & 1;
                    cpu->v[x] >>= 1;
                    break;
                }
                case 0x7:
                {
                    // 0x8XY7: set vx to vy minus vx. vf is set to 0 when there is borrow, and 1 when not
                    int16_t result16 = (int)cpu->v[y] - (int)cpu->v[x];
                    cpu->v[y] = (uint8_t)result16;
                    cpu->v[0xF] = result16 >= 0 ? 1 : 0;
                    break;
                }
                case 0xE:
                {
                    // 0x8XYE: shift vx left by one, vf is set to the msb of vx before the shift
                    uint8_t vx = cpu->v[x];
                    cpu->v[0xF] = vx >> 7;
                    cpu->v[x] <<= 1;
                    break;
                }
                default:
                {
                    goto illegal_op;
                    break;
                }
            }
            break;
        }
        case 0x9000:
        {
            // 0x9XY0: skip the next instruction if vx doesnt equal vy
            if (cpu->v[x] != cpu->v[y])
            {
                cpu->pc += 2;
            }
            break;
        }
        case 0xA000:
        {
            // 0xANNN: set I to the address NNN
            cpu->i = nnn;
            break;
        }
        case 0xB000:
        {
            // 0xBNNN: jump to the address NNN plus V0
            cpu->pc = nnn + cpu->v[0];
            break;
        }
        case 0xC000:
        {
            // 0xCXNN: set VX to the result of bitwise AND between NN and rand(0,255)
            int prn = (rand() % 0x100);
            cpu->v[x] = (prn & nn);
            break;
        }
        case 0xD000:
        {
            // 0xDXYN: sprite drawing
            break;
        }
        case 0xE000:
        {
            switch (op & 0xFF)
            {
                case 0x9E:
                {
                    // 0xEX9E: skip the next instruction if the key stored in vx is pressed
                    uint8_t key = cpu->v[x];
                    if (key_pressed(cpu->c8, key))
                    {
                        cpu->pc += 2;
                    }
                    break;
                }
                case 0xA1: 
                {
                    // 0xEXA1: skip the next instruction if the key stored in vx is not pressed
                    uint8_t key = cpu->v[x];
                    if (!key_pressed(cpu->c8, key))
                    {
                        cpu->pc += 2;
                    }
                    break;
                }
                default:
                {
                    goto illegal_op;
                    break;
                }
            }
            break;
        }
        case 0xF000:
        {
            switch (op & 0xFF)
            {
                case 0x07:
                {
                    // 0xFX07: set vx to the value of the delay timer
                    cpu->v[x] = cpu->timer_delay;
                    break;
                }
                case 0x0A:
                    // 0xFX0A: a key press is awaited, then stored in vx
                    cpu->v[x] = await_key(cpu->c8);
                    break;
                case 0x15:
                {
                    // 0xFX15: set the delay timer to vx
                    cpu->timer_delay = cpu->v[x];
                    break;
                }
                case 0x18:
                {
                    // 0xFX18: set the sound timer to vx
                    cpu->timer_sound = cpu->v[x];
                    break;
                }
                case 0x1E:
                {
                    // 0xFX1E: add vx to i
                    cpu->i += cpu->v[x];
                    break;
                }
                case 0x29:
                {
                    // 0xFX29: set i to the location of the sprite character stored in x
                    cpu->i = cpu->v[x] * 5;
                    break;
                }
                case 0x33:
                {
                    // 0xFX33: store BCD repreentation of vx in memory locations i, i+1, i+2
                    uint8_t vx = cpu->v[x];
                    mem_write8(cpu->c8, cpu->i, vx / 100);
                    mem_write8(cpu->c8, cpu->i + 1, (vx / 10) % 10);
                    mem_write8(cpu->c8, cpu->i + 2, vx % 10);
                    break;
                }
                case 0x55:
                {
                    // 0xFX55: store registers v0 through vx in memory starting at location I
                    assert(x >= 0);
                    assert(cpu->i >= C8_LOAD_ADDR);
                    for (int i= 0; i <= x; i++)
                    {
                        uint16_t dst = mem_read8(cpu->c8, cpu->i + i);
                        mem_write8(cpu->c8, dst, cpu->v[i]);
                    }
                    break;
                }
                case 0x65:
                {
                    // 0xFX65: fill v0 to vx (including vx) with values from memory starting at i
                    assert(x <= 0xF);
                    assert(cpu->i >= C8_LOAD_ADDR);
                    for (int i = 0; i <= x; i++)
                    {
                        cpu->v[i] = mem_read8(cpu->c8, cpu->i + i);
                    }
                    break;
                }
                default:
                {
                    goto illegal_op;
                    break;
                }
            }
            break;
        }
        default:
        {
            goto illegal_op;
            break;
        }

illegal_op:
        fprintf(stderr, "Illegal op: %#04x\n", op); 
        exit(EXIT_FAILURE);
    }


    // Decrement timers
    if (cpu->timer_delay)
    {
        cpu->timer_delay--;
    }
    if (cpu->timer_sound)
    {
        cpu->timer_sound--;
        if (cpu->timer_sound == 0)
        {
            printf("####BEEP####\n");
            // TODO: play a sound
        }
    }
}

uint16_t pop(struct c8_cpu *cpu)
{
    assert(cpu->sp > 0);
    uint16_t result = cpu->stack[cpu->sp - 1];
    cpu->sp--;
    return result;
}

void push(struct c8_cpu *cpu, uint16_t value)
{
    assert(cpu->sp <= sizeof cpu->stack);
    cpu->stack[cpu->sp] = value;
    cpu->sp++;
}

void cpu_print(struct c8_cpu *cpu)
{
    printf("[PC:%#04x, SP:%#04x, I:%#04x, OP:%#04x]\n", 
            cpu->pc, cpu->sp, cpu->i, mem_read16(cpu->c8, cpu->pc));
    for (int i = 0; i <= 0xF; i++)
    {
        printf("\tv%x:%04x    s%x:%#04x\n", i, cpu->v[i], i, cpu->stack[i]);
    }
    printf("------------------------------------\n");
}
