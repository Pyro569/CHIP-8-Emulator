/*
    This part of the emulator exists strictly for configuring the emulator to be how you want it to be
    ex: keybinds, timings, graphics etc
*/

#include <string.h>
#include <stdio.h>

typedef struct
{
    int ispsLimit;
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
            fprintf(file, "%s", "ispsLimit=700\nsuper=0");
            fclose(file);
            parseINI("config.ini");
        }
        else
        {
            perror("Error creating config file");
            exit(1);
        }
    }

    // do the tokenization
    printf("Found emulator config file: \n\n");

    // initialize important stuffs
    char *token = malloc(255);
    int tokenIndex = 0;
    char *_tokens[255];
    int tokenCount = 0;

    for (int i = 0; buffer[i] != '\0'; i++)
    {
        printf("%c", buffer[i]);
        if (buffer[i] != '=')
        {
            token[tokenIndex] = buffer[i];
            tokenIndex++;
        }
        if (buffer[i + 1] == '\0' || buffer[i] == '=')
        {
            _tokens[tokenCount] = malloc((tokenIndex + 1) * sizeof(char));
            strcpy(_tokens[tokenCount], token);
            tokenCount++;

            memset(token, 0, sizeof(token));
            tokenIndex = 0;
        }
    }
    printf("\n\n");
    free(buffer);

    for (int i = 0; i < tokenCount; i++)
        tokens[i] = _tokens[i];
}

void configureEmulator(iniSettings *_iniSettings)
{
    for (int i = 0; tokens[i] != NULL; i++)
    {
        if (strcmp(tokens[i], "ispsLimit") == 0)
        {
            if (tokens[i + 1] != NULL)
            {
                _iniSettings->ispsLimit = atoi(tokens[i + 1]);
            }
            else
            {
                _iniSettings->ispsLimit = 700;
            }
        }
    }
}