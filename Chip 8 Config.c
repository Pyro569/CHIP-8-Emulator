/*
    This part of the emulator exists strictly for configuring the emulator to be how you want it to be
    ex: keybinds, timings, graphics etc
*/

#include <string.h>
#include <stdio.h>

typedef struct
{
    int ispsLimit;
    int super;
    int offR;
    int offG;
    int offB;
    int onR;
    int onG;
    int onB;
} iniSettings;

char *tokens[255];

void parseINI(char *filename)
{
    // read the file or create file if needed
    FILE *file = fopen(filename, "rb");
    char *buffer = NULL;
    long length;

    if (file)
    {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = malloc(length + 1);
        if (buffer)
        {
            fread(buffer, 1, length, file);
            buffer[length] = '\0';
        }
        fclose(file);
    }
    else
    {
        FILE *file = fopen("config.ini", "w");
        if (file)
        {
            fprintf(file, "%s", "ispsLimit=700\nsuper=0\noffRGB=0 0 0\nonRGB=1 1 1");
            fclose(file);
            usleep(3000000);
            parseINI("config.ini");
        }
        else
        {
            perror("Error creating config file");
            exit(1);
        }
    }

    // tokenize the ini file by spaces, equals and null terminations
    char *token;
    int tokenCount = 0;

    token = strtok(buffer, "= \n");
    while (token != NULL)
    {
        tokens[tokenCount] = malloc(strlen(token) + 1);
        strcpy(tokens[tokenCount], token);
        tokenCount++;
        token = strtok(NULL, "= \n");
    }
    tokens[tokenCount] = NULL;
    free(buffer);
}

void configureEmulator(iniSettings *_iniSettings)
{
    for (int i = 0; tokens[i] != NULL; i++)
    {
        if (strcmp(tokens[i], "ispsLimit") == 0)
        {
            if (tokens[i + 1] != NULL)
                _iniSettings->ispsLimit = atoi(tokens[i + 1]);
            else
                _iniSettings->ispsLimit = 700;
        }
        else if (strcmp(tokens[i], "super") == 0)
        {
            if (tokens[i + 1] != NULL)
                _iniSettings->super = atoi(tokens[i + 1]);
            else
                _iniSettings->super = 0;
        }
        else if (strcmp(tokens[i], "offRGB") == 0)
        {
            if (tokens[i + 3] != NULL)
            {
                _iniSettings->offR = atoi(tokens[i + 1]);
                _iniSettings->offG = atoi(tokens[i + 2]);
                _iniSettings->offB = atoi(tokens[i + 3]);
                printf("%d", _iniSettings->offB);
            }
            else
            {
                _iniSettings->offR = 0;
                _iniSettings->offG = 0;
                _iniSettings->offB = 0;
            }
        }
        else if (strcmp(tokens[i], "onRGB") == 0)
        {
            if (tokens[i + 3] != NULL)
            {
                _iniSettings->onR = atoi(tokens[i + 1]);
                _iniSettings->onG = atoi(tokens[i + 2]);
                _iniSettings->onB = atoi(tokens[i + 3]);
            }
            else
            {
                _iniSettings->onR = 1;
                _iniSettings->onG = 1;
                _iniSettings->onB = 1;
            }
        }
    }
}