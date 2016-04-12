#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "main.h"
#include "chip8.h"
#include "cpu.h"

/* The system instance. */
struct chip8 c8;
struct c8_cpu cpu;

void sig_handler(int sig)
{
    c8_destroy(&c8);
    exit(EXIT_SUCCESS);
}

void load_rom_file(char *filename, struct chip8 *c8)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        perror("Failed to read rom file");
        exit(EXIT_FAILURE);
    }

    ssize_t max_size = C8_MAX_ADDR - C8_LOAD_ADDR;
    uint8_t *mem_ptr = &c8->memory[C8_LOAD_ADDR];
    ssize_t bytes_read = 0;
    while(fread(mem_ptr + bytes_read, 1, 1, f))
    {
        bytes_read++;
        if (bytes_read >=  (max_size - 1))
        {
            fprintf(stderr, "Rom too large\n");
            fclose(f);
            exit(EXIT_FAILURE);
        }
    }
    printf("Rom bytes read: %zu\n", bytes_read);
    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <romfile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_handler);
    c8_init(&c8, &cpu);
    load_rom_file(argv[1], &c8);
    c8_run(&c8);

    return 0;
}
