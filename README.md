So far the emulator can successfully boot the IBM Logo ROM and pass most Corax opcode checks.  
IO is not implemented fully yet as I suck at OpenGL and am trying to figure that out.  
Can be compiled on Linux with the command "gcc "Chip 8 Emulator.c" -o Chip8 -lGL -lglut"  
Runs ROMs on Linux by taking in the first command line arg as the ROM to boot.  
Emulator can be configured via config.ini value, only existing editable value currently is ispsLimit (instructions per second limit), default is 700
