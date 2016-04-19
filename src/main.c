#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "cpu.h"

// The system instance
struct chip8 c8;
struct c8_cpu cpu;

void sig_handler(int sig)
{
    fprintf(stderr, "Received SIGINT\n");
    if (c8.alive)
    {
        c8_destroy(&c8);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <romfile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        if (signal(SIGINT, &sig_handler) == SIG_ERR)
        {
            fprintf(stderr, "Failed to register shutdown hook\n");
            exit(EXIT_FAILURE);
        }

        if (!c8_init(&c8, &cpu))
        {
            fprintf(stderr, "Failed to init CHIP-8 system\n");
            exit(EXIT_FAILURE);
        }

        if (c8_load(argv[1], &c8, C8_LOAD_ADDR) == -1)
        {
            fprintf(stderr, "Failed to load '%s'\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    if (c8_run(&c8, C8_LOAD_ADDR) != 0)
    {
        fprintf(stderr, "CHIP-8 terminated unexpectedly\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
