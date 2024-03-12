using System.Diagnostics;

namespace Chip8
{
    class Emulator
    {
        static string program = "";
        static int pc = 0;
        static string programContent = "";
        static string currentOpcode;

        public static bool emulating;

        public static List<List<int>> screen = new List<List<int>>();

        public static void Main(string[] args)
        {
            CPU cpu = new CPU();
            emulating = true;

            //get the binary input as an argument and read the contents of the binary
            program = args[0];
            StreamReader sr = new StreamReader(program);
            programContent = sr.ReadToEnd();

            //hexdump the binary
            byte[] bytes = System.Text.Encoding.UTF8.GetBytes(programContent);
            programContent = Convert.ToHexString(bytes);

            //create the screen list through some magic math
            for (int i = 0; i < 32; i++)
            {
                List<int> row = new List<int>();
                for (int k = 0; k < 65; k++)
                    row.Add(0);
                screen.Add(row);
            }

            replacePixel(0, 64, 1);

            while (emulating)
            {
                Console.Clear();
                drawScreen();

                for (int i = 0; i < programContent.Length; i++)
                {
                    currentOpcode = programContent[i] + "" + programContent[i + 1];
                    //currentOpcode += programContent[i + 1];
                    //increment i to account for the extra pc counter & increment pc
                    i += 1;
                    pc += 2;

                    //decodeOpCode(currentOpcode);
                }

                //set a stopwatch timer so that way the emulator is locked at 60fps
                Stopwatch timer = new Stopwatch();
                timer.Start();
                while (timer.Elapsed < TimeSpan.FromSeconds(1000 / 60)) ;
            }
        }

        public static void decodeOpCode(string opcode)
        {
            switch (opcode)
            {
                case "E0":
                    Console.Clear();
                    Console.WriteLine("Clearing screen...");
                    break;
            }
        }

        public static void drawScreen()
        {
            for (int i = 0; i < screen.Count; i++)
            {
                for (int k = 0; k < screen[i].Count; k++)
                    if (screen[i][k] == 1)
                        Console.Write("■");
                    else
                        Console.Write(" ");
                Console.Write("\n");
            }
        }

        public static void replacePixel(int px, int py, int status)
        {
            screen[px][py] = status;
        }
    }
}