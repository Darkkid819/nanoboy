#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gameboy.h"
#include "utils.h"

typedef enum {
    HELP,
    STEP,
    RUN,
    INVALID
} Command;

Command validargs(int argc, char *argv[], int *cycles, char **romPath) {
    if (argc < 2) {
        return INVALID;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        return HELP;
    } else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--step") == 0) {
        if (argc >= 4) {
            *cycles = atoi(argv[2]);
            *romPath = argv[3];
            return STEP;
        } else {
            return INVALID;
        }
    } else if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--run") == 0) {
        if (argc >= 3) {
            *romPath = argv[2]; 
            return RUN;
        } else {
            return INVALID;
        }
    } else {
        return INVALID;
    }
}

int main(int argc, char *argv[]) {
    int cycles = 0; 
    char *romPath = NULL;

    Command cmd = validargs(argc, argv, &cycles, &romPath);

    switch (cmd) {
        case HELP:
            USAGE(argv[0], EXIT_SUCCESS);
            break;

        case STEP:
            {
                GameBoy gameBoy;
                initGameBoy(&gameBoy);
                if (loadGameBoyROM(&gameBoy, romPath) != 0) {
                    error("Failed to load ROM");
                    return EXIT_FAILURE;
                }
                stepGameBoy(&gameBoy, cycles);
            }
            break;

        case RUN:
            {
                GameBoy gameBoy;
                initGameBoy(&gameBoy);
                if (loadGameBoyROM(&gameBoy, romPath) != 0) {
                    error("Failed to load ROM");
                    return EXIT_FAILURE;
                }
                runGameBoy(&gameBoy);
            }
            break;

        case INVALID:
        default:
            error("Invalid arguments.");
            USAGE(argv[0], EXIT_FAILURE);
            break;
    }

    return EXIT_SUCCESS;
}
