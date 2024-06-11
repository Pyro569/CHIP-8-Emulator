#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <string.h>
#include <unistd.h>
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
    uint8_t keys[16];
    uint8_t waiting_for_key;
    uint8_t keyRegister;
    uint8_t delayTimer;
    uint8_t soundTimer;
} Chip8;

void setPixel(int x, int y)
{
    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    glVertex2i(x, y);
    glVertex2i(x + 1, y);
    glVertex2i(x + 1, y - 1);
    glVertex2i(x, y - 1);
    glVertex2i(x, y);
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
            glClear(GL_COLOR_BUFFER_BIT);
            break;
        case 0xee:
            printf("RET");
            uint16_t target = (chip8->memory[chip8->sp] << 8) | chip8->memory[chip8->sp + 1];
            chip8->sp += 2;
            pc = target;
            break;
        }
        break;
    case 0x1:
        printf("JUMP   $%01x%02x", code[0] & 0xf, code[1]);
        uint16_t target = ((code[0] & 0xf) << 8) | code[1];
        if (target == pc)
        {
            printf("\nINFINITE LOOP  HALTING");
            chip8->halt = 1;
        }
        pc = target;
        break;
    case 0x2:
        chip8->sp -= 2;
        memory[chip8->sp] = ((pc + 2) & 0xFF00) >> 8;
        memory[chip8->sp + 1] = (pc + 2) & 0xFF;
        pc = ((code[0] & 0xf) << 8) | code[1];
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
        uint8_t reg1 = code[0] & 0xf;
        uint8_t reg2 = (code[1] & 0xf0) >> 4;
        if (chip8->V[reg1] != chip8->V[reg2])
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
        printf("%-6s V%01X, V%01X, $%01x", "SPRITE", code[0] & 0xf, code[1] >> 4, code[1] & 0xf);
        int x = chip8->V[code[0] & 0xf];
        int y = chip8->V[(code[1] & 0xf0) >> 4];

        chip8->V[0xF] = 0;
        int height = code[1] & 0xf;
        for (int yline = 0; yline < height; yline++)
        {
            uint8_t pixel = memory[chip8->I + yline];
            for (int i = 0; i < 8; i++)
            {
                if (pixel & (0x80 >> i))
                {
                    int screenX = x + i;
                    int screenY = y - yline + height;
                    setPixel(screenX, screenY);
                }
            }
        }

        break;
    case 0xe:
        switch (code[1])
        {
        case 0x9e:
            if (chip8->keys[code[0] & 0xf] == 1)
            {
                opbytes += 2;
            }
            break;
        case 0xa1:
            if (chip8->keys[code[0] & 0xf] == 0)
            {
                opbytes += 2;
            }
            break;
        }
        break;
    case 0xf:
        reg = code[0] & 0xf;
        switch (code[1])
        {
        case 0x0a:
            chip8->waiting_for_key = 1;
            chip8->keyRegister == code[0] & 0xf;
            while (chip8->waiting_for_key == 1)
            {
            }
            break;
        case 0x15:
            chip8->delayTimer = code[0] & 0xf;
            break;
        case 0x18:
            chip8->soundTimer = code[0] & 0xf;
            break;
        case 0x1e:
            chip8->I = chip8->I + chip8->V[code[0] & 0xf];
            break;
        case 0x29:
            printf("SPRITECHAR");
            chip8->I = FONT_BASE + (chip8->V[code[1] & 0xf]);
            break;
        case 0x65:
            uint8_t reg = code[0] & 0xf;
            for (int i = 0; i <= reg; i++)
                chip8->V[i] = chip8->memory[chip8->I + i];
            chip8->I += (reg + 1);
            break;
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
Chip8 *_chip8;

void *emulatorDisplay(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

    Chip8 *chip8 = malloc(sizeof(chip8));
    _chip8 = chip8;

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

    int pc = 0x200;
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

    uint32_t timeSlept = 0;

    while (pc < pos && chip8->halt != 1)
    {
        pc += Emulate(chip8, chip8->memory, pc, chip8->memory, pos);
        usleep(1000000 / 700);
        timeSlept += 1000000 / 700;
        if (timeSlept % 1000000 == 0)
        {
            chip8->delayTimer -= 60;
            chip8->soundTimer -= 60;
        }
    }
}

void keyDown(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 13:
        printf("Enter Down\n");
        _chip8->keys[0xb] = 1;
        break;
    case 42:
        printf("* Down\n");
        _chip8->keys[0xd] = 1;
        break;
    case 43:
        printf("+ Down\n");
        _chip8->keys[0xf] = 1;
        break;
    case 45:
        printf("- Down\n");
        _chip8->keys[0xe] = 1;
        break;
    case 46:
        printf(". Down\n");
        _chip8->keys[0xa] = 1;
        break;
    case 47:
        printf("/ Down\n");
        _chip8->keys[0xc] = 1;
        break;
    case 48:
        printf("0 Down\n");
        _chip8->keys[0] = 1;
        break;
    case 49:
        printf("1 Down\n");
        _chip8->keys[1] = 1;
        break;
    case 50:
        printf("2 Down\n");
        _chip8->keys[2] = 1;
        break;
    case 51:
        printf("3 Down\n");
        _chip8->keys[3] = 1;
        break;
    case 52:
        printf("4 Down\n");
        _chip8->keys[4] = 1;
        break;
    case 53:
        printf("5 Down\n");
        _chip8->keys[5] = 1;
        break;
    case 54:
        printf("6 Down\n");
        _chip8->keys[6] = 1;
        break;
    case 55:
        printf("7 Down\n");
        _chip8->keys[7] = 1;
        break;
    case 56:
        printf("8 Down\n");
        _chip8->keys[8] = 1;
        break;
    case 57:
        printf("9 Down\n");
        _chip8->keys[9] = 1;
        break;
    }

    if (_chip8->waiting_for_key == 1)
    {
        _chip8->V[_chip8->keyRegister] = key;
        _chip8->waiting_for_key = 0;
    }
}

void keyUp(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 13:
        printf("Enter Up\n");
        _chip8->keys[0xb] = 0;
        break;
    case 42:
        printf("* Up\n");
        _chip8->keys[0xd] = 0;
        break;
    case 43:
        printf("+ Up\n");
        _chip8->keys[0xf] = 0;
        break;
    case 45:
        printf("- Up\n");
        _chip8->keys[0xe] = 0;
        break;
    case 46:
        printf(". Up\n");
        _chip8->keys[0xa] = 0;
        break;
    case 47:
        printf("/ Up\n");
        _chip8->keys[0xc] = 0;
        break;
    case 48:
        printf("0 Up\n");
        _chip8->keys[0] = 0;
        break;
    case 49:
        printf("1 Up\n");
        _chip8->keys[1] = 0;
        break;
    case 50:
        printf("2 Up\n");
        _chip8->keys[2] = 0;
        break;
    case 51:
        printf("3 Up\n");
        _chip8->keys[3] = 0;
        break;
    case 52:
        printf("4 Up\n");
        _chip8->keys[4] = 0;
        break;
    case 53:
        printf("5 Up\n");
        _chip8->keys[5] = 0;
        break;
    case 54:
        printf("6 Up\n");
        _chip8->keys[6] = 0;
        break;
    case 55:
        printf("7 Up\n");
        _chip8->keys[7] = 0;
        break;
    case 56:
        printf("8 Up\n");
        _chip8->keys[8] = 0;
        break;
    case 57:
        printf("9 Up\n");
        _chip8->keys[9] = 0;
        break;
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
    glOrtho(0, 64, 0, 32, -1.0, 1.0);
    glutDisplayFunc(emulatorDisplay(argc, argv));
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutMainLoop();
}

int main(int argc, char **argv)
{
    initWindow(argc, argv);
    return 0;
}