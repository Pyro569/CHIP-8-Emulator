#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <string.h>
#include <unistd.h>

#include "Chip 8 Config.c"
#include "Chip 8 Font.c"

#define FONT_BASE 0
#define FONT_SIZE 5 * 16

typedef struct Chip8
{
    uint8_t V[16];
    int pc;
    int pos;
    uint16_t sp;
    uint16_t I;
    uint8_t halt;
    uint8_t *memory;
    uint8_t keys[16];
    uint8_t waiting_for_key;
    uint8_t keyRegister;
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint32_t timeSlept;
    iniSettings _iniSettings;
} Chip8;

Chip8 *_chip8;
iniSettings *_configuredINI;
uint8_t screen[64][32];

int isPixelSet(int x, int y)
{
    return screen[x % 64][y % 32];
}

void setPixel(int x, int y)
{
    int wrappedX = x % 64;
    int wrappedY = y % 32;

    // Toggle the pixel state
    screen[wrappedX][wrappedY] ^= 1;

    if (screen[wrappedX][wrappedY])
    {
        // Set pixel color if the pixel is now on
        glColor3f(_chip8->_iniSettings.onR, _chip8->_iniSettings.onG, _chip8->_iniSettings.onB);
    }
    else
    {
        // Clear pixel color if the pixel is now off
        glColor3f(0.0f, 0.0f, 0.0f); // Assuming black is the background color
    }

    // Render the pixel
    glBegin(GL_POLYGON);
    glVertex2i(wrappedX, wrappedY);
    glVertex2i(wrappedX + 1, wrappedY);
    glVertex2i(wrappedX + 1, wrappedY - 1);
    glVertex2i(wrappedX, wrappedY - 1);
    glVertex2i(wrappedX, wrappedY);
    glEnd();

    glFlush();
}

int Emulate(Chip8 *chip8, uint8_t *codebuffer, int pc, unsigned char *memory, int pos)
{
    unsigned char *code = &codebuffer[pc];
    uint8_t firstnib = (code[0] >> 4);
    int opbytes = 2;

    uint8_t reg;
    uint8_t reg1;
    uint8_t reg2;

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
            chip8->pc = target;
            break;
        }
        break;
    case 0x1:
        printf("JUMP   $%01x%02x", code[0] & 0xf, code[1]);
        uint16_t target = ((code[0] & 0x0F) << 8) | code[1];
        if (target == pc)
        {
            printf("\nINFINITE LOOP  HALTING");
            chip8->halt = 1;
        }
        int leftovers = target - pc;
        opbytes = leftovers;
        break;
    case 0x2:
        chip8->sp -= 2;
        chip8->memory[chip8->sp] = (pc & 0xFF00) >> 8;
        chip8->memory[chip8->sp + 1] = pc & 0xFF;
        uint16_t targ = ((code[0] & 0xf) << 8) | code[1];
        opbytes = targ - pc;
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
        reg1 = code[0] & 0xf;
        reg2 = (code[1] & 0xf0) >> 4;
        if (chip8->V[reg1] == chip8->V[reg2])
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
        reg1 = code[0] & 0xf;
        reg2 = (code[1] & 0xf0) >> 4;
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
        uint8_t x = chip8->V[code[0] & 0xf];
        uint8_t y = chip8->V[(code[1] & 0xf0) >> 4];
        uint8_t height = code[1] & 0xf;
        uint8_t pixel;

        chip8->V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = chip8->memory[chip8->I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    int screenX = (x + xline) % 64;
                    int screenY = (y + yline) % 32;

                    if (isPixelSet(screenX, screenY))
                        chip8->V[0xF] = 1;

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
                opbytes = 4;
            break;
        case 0xa1:
            if (chip8->keys[code[0] & 0xf] == 0)
                opbytes = 4;
            break;
        }
        break;
    case 0xf:
        reg = code[0] & 0xf;
        switch (code[1])
        {
        case 0x07:
            chip8->V[code[0] & 0xf] = chip8->delayTimer;
            break;
        case 0x0a:
            chip8->waiting_for_key = 1;
            chip8->keyRegister = code[0] & 0xf;
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
        case 0x33:
            uint8_t ones, tens, hundreds;
            uint8_t value = chip8->V[reg];
            ones = value % 10;
            value = value / 10;
            tens = value % 10;
            hundreds = value / 10;
            chip8->memory[chip8->I] = hundreds;
            chip8->memory[chip8->I + 1] = tens;
            chip8->memory[chip8->I + 2] = ones;
            break;
        case 0x55:
            int i;
            reg = code[0] & 0xf;
            for (i = 0; i <= reg; i++)
                memory[chip8->I + i] = chip8->V[i];
            chip8->I += (reg + 1);
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

void *emulatorDisplay(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

    Chip8 *chip8 = malloc(sizeof(Chip8));

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

    chip8->pc = 0x200;
    chip8->sp = 0xfa0;
    chip8->memory = calloc(1024 * 4, 1);
    if (chip8->memory == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        free(chip8);
        fclose(file);
        exit(0);
    }
    memcpy(&chip8->memory[FONT_SIZE], font4x5, FONT_SIZE);

    chip8->pos = 0x200;
    while (fread(&chip8->memory[chip8->pos], 1, 1, file))
    {
        chip8->pos++;
    }
    fclose(file);

    iniSettings _iniSettings;
    parseINI("config.ini");
    configureEmulator(&_iniSettings);
    _configuredINI = &_iniSettings;
    chip8->_iniSettings = *_configuredINI;

    chip8->timeSlept = 0;

    glClearColor((float)_configuredINI->offR, (float)_configuredINI->offG, (float)_configuredINI->offB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    _chip8 = chip8;
}

void display()
{
    glClearColor((float)_chip8->_iniSettings.offR, (float)_chip8->_iniSettings.offG, (float)_chip8->_iniSettings.offB, 1.0f);
    glColor3f(_chip8->_iniSettings.onR, _chip8->_iniSettings.onG, _chip8->_iniSettings.onB);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void emulateLoop()
{
    if (_chip8->pc < _chip8->pos && _chip8->halt != 1)
    {
        _chip8->pc += Emulate(_chip8, _chip8->memory, _chip8->pc, _chip8->memory, _chip8->pos);
        usleep(1000000 / _chip8->_iniSettings.ispsLimit);
    }
}

void frameDrawn()
{
    _chip8->delayTimer -= 1;
    _chip8->soundTimer -= 1;
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
    emulatorDisplay(argc, argv);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 320);
    glutInitWindowPosition(GLUT_SCREEN_WIDTH / 2, GLUT_SCREEN_HEIGHT / 2);
    glutCreateWindow("Pyro569's Chip-8 Emulator");
    glLoadIdentity();
    glOrtho(0, 64, 32, 0, -1.0, 1.0);
    glutTimerFunc(1000 / 60, frameDrawn, 0);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutDisplayFunc(display);
    glutIdleFunc(emulateLoop);
    glutMainLoop();
}

int main(int argc, char **argv)
{
    initWindow(argc, argv);
    return 0;
}