#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <string.h>
#include "Chip 8 Font.c"

#define FONT_BASE 0
#define FONT_SIZE 5 * 16

typedef struct Chip8
{
    uint8_t V[16];
    uint16_t sp;
    uint16_t I;
    uint8_t halt;
    uint8_t *screen;
    uint8_t *memory;
} Chip8;

void setPixel(int x, int y)
{
    // glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    glVertex2i(x * 10, y * 10);
    glVertex2i(x * 10 + 10, y * 10);
    glVertex2i(x * 10 + 10, y * 10 - 10);
    glVertex2i(x * 10, y * 10 - 10);
    glVertex2i(x * 10, y * 10);
    glEnd();

    glFlush();
}

int Emulate(Chip8 *chip8, uint8_t *codebuffer, int pc, unsigned char *memory, int pos)
{
    unsigned char *code = &codebuffer[pc];
    uint8_t firstnib = (code[0] >> 4);
    int opbytes = 2;

    uint8_t reg;

    printf("%04x %02x %02x     ", pc, code[0], code[1]);
    switch (firstnib)
    {
    case 0x0:
        switch (code[1])
        {
        case 0xe0:
            printf("CLS");
            // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 0xee:
            printf("RET");
            break;
        default:
            printf("Unknown 0 opcode");
            break;
        }
        break;
    case 0x1:
        printf("JUMP   $%01x%02x", code[0] & 0xf, code[1]);
        if (pc == ((code[0] & 0xf) << 8) | code[1])
        {
            printf("\nINF LOOP       HALTING\n");
            chip8->halt = 1;
            while (1)
            {
            }
        }
        pc = ((code[0] & 0xf) << 8) | code[1];
        opbytes = 0;
        break;
    case 0x2:
        chip8->sp -= 2;
        memory[chip8->sp] = ((pc + 2) & 0xFF00) >> 8;
        memory[chip8->sp + 1] = (pc + 2) & 0xFF;
        pc = ((code[0] & 0x0f) << 8) | code[1];
        break;
    case 0x3:
        printf("JMP    EQ");
        if (chip8->V[code[0] & 0xf] == code[1])
            opbytes = 4;
        break;
    case 0x4:
        printf("JMP    NE");
        if (chip8->V[code[0] & 0xf] != code[1])
            opbytes = 4;
        break;
    case 0x5:
        printf("JMP    EQ");
        if (chip8->V[code[0] & 0xf] == chip8->V[(code[1] & 0xf0) >> 4])
            opbytes = 4;
        break;
    case 0x6:
        reg = code[0] & 0x0f;
        printf("MOV    V%01x, $%02x", reg, code[1]);
        chip8->V[reg] = code[1];
        break;
    case 0x7:
        reg = code[0] & 0x0f;
        printf("ADD    V%01x, $%02x", reg, code[1]);
        chip8->V[reg] += code[1];
        break;
    case 0x8:
        uint8_t vx = code[0] & 0xf;
        uint8_t vy = (code[1] & 0xf0) >> 4;
        uint8_t vf;
        int borrow;

        switch (code[1] & 0xf)
        {
        case 0:
            printf("MOV    $%01x, $%01x", vx, vy);
            chip8->V[vx] = chip8->V[vy];
            break;
        case 1:
            printf("OR     $%01x, $0%1x", vx, vy);
            chip8->V[vx] |= chip8->V[vy];
            break;
        case 2:
            printf("AND    $%01x, $0%1x", vx, vy);
            chip8->V[vx] &= chip8->V[vy];
            break;
        case 3:
            printf("XOR    $%01x, $0%1x", vx, vy);
            chip8->V[vx] ^= chip8->V[vy];
            break;
        case 4:
            printf("INC    $%01x, $%01x", vx, vy);
            uint16_t res = chip8->V[vx] + chip8->V[vy];
            if (res & 0xff00)
                chip8->V[0xf] = 1;
            else
                chip8->V[0xf] = 0;
            chip8->V[vx] = res & 0xff;
            break;
        case 5:
            printf("DEC    $%01x, $%01x", vx, vy);
            borrow = (chip8->V[vx] > chip8->V[vy]);
            chip8->V[vx] -= chip8->V[vy];
            chip8->V[0xf] = borrow;
            break;
        case 6:
            vf = chip8->V[vx] & 0x1;
            chip8->V[vx] = chip8->V[vx] >> 1;
            chip8->V[0xf] = vf;
            break;
        case 7:
            borrow = (chip8->V[vy] > chip8->V[vx]);
            chip8->V[vx] = chip8->V[vy] - chip8->V[vx];
            chip8->V[0xf] = borrow;
            break;
        case 0xe:
            vf = (0x80 == (chip8->V[vx] & 0x80));
            chip8->V[vx] = chip8->V[vx] << 1;
            chip8->V[0xf] = vf;
            break;
        }
        break;
    case 0x9:
        printf("JMP    NE");
        if (code[0] & 0xf != (code[1] & 0xf0) >> 4)
        {
            opbytes = 4;
        }
        break;
    case 0xa:
        printf("MOV    I, $%01x%02x", code[0] & 0xf, code[1]);
        chip8->I = ((code[0] & 0xf) << 8) | code[1];
        break;
    case 0xb:
        printf("JMP    ($%01x%02x + V0)", code[0] & 0xf, code[1]);
        pc = ((uint16_t)chip8->V[0] + (((code[0] & 0xf) << 8) | code[1]));
        break;
    case 0xc:
        printf("RAND");
        reg = code[0] & 0xf;
        chip8->V[reg] = random() & code[1];
        break;
    case 0xd:
        printf("%-6s V%01X, V%01X, $%01x\n", "SPRITE", code[0] & 0xf, code[1] >> 4, code[1] & 0xf);
        int x = chip8->V[code[0] & 0xf];
        int y = chip8->V[(code[1] & 0xf0) >> 4];

        chip8->V[0xF] = 0;

        printf("VX: %d, VY: %d, I: %d", x, y, chip8->I);
        int height = code[1] & 0xf;
        for (int yline = 0; yline < height; yline++)
        {
            uint8_t pixel = memory[chip8->I + yline];
            for (int i = 7; i >= 0; i--)
            {
                if (pixel & (1 << i))
                {
                    int screenX = x - i;
                    int screenY = y - yline;

                    setPixel(screenX - 32, screenY);
                }
            }
        }

        break;
    case 0xe:
        printf("e not implemented yet");
        break;
    case 0xf:
        reg = code[0] & 0xf;
        switch (code[1])
        {
        case 0x29:
            printf("SPRITECHAR");
        }
        break;
    default:
        printf("Unrecognized opcode");
        exit(0);
        break;
    }

    printf("\n");

    return opbytes;
}

void *emulatorDisplay(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

    Chip8 *chip8 = malloc(sizeof(chip8));

    if (chip8 == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        exit(0);
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf("Error: Unable to open file\n");
        free(chip8);
        exit(0);
    }

    int pc = 0x200; // 0x200;
    chip8->sp = 0xfa0;
    chip8->memory = calloc(1024 * 4, 1);
    memcpy(&chip8->memory[FONT_SIZE], font4x5, FONT_SIZE);

    int pos = 0x200;
    while (fread(&chip8->memory[pos], 1, 1, file))
    {
        pos++;
    }
    fclose(file);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    while (pc < pos && chip8->halt != 1)
    {
        pc += Emulate(chip8, chip8->memory, pc, chip8->memory, pos);
    }
}

void initWindow(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 320);
    glutInitWindowPosition(GLUT_SCREEN_WIDTH / 2, GLUT_SCREEN_HEIGHT / 2);
    glutCreateWindow("Pyro569's Chip-8 Emulator");
    glLoadIdentity();
    glOrtho(-320, 320, -160, 160, -1.0, 1.0);
    glutDisplayFunc(emulatorDisplay(argc, argv));
    glutMainLoop();
}

int main(int argc, char **argv)
{
    initWindow(argc, argv);
    return 0;
}