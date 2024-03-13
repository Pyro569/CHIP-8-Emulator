using System.Diagnostics;

namespace Chip8
{
    class Emulator
    {
        static string program = "";
        static int pc = 0;
        static string programContent = "";
        static string currentOpcode;

        static int targettedFPS = 60;

        public static bool emulating;

        public static List<List<int>> screen = new List<List<int>>();
        public static List<string> OpCodes = new List<string>();
        static List<string> devOutputs = new List<string>();

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
            for (int i = 0; i < 33; i++)
            {
                List<int> row = new List<int>();
                for (int k = 0; k < 65; k++)
                    row.Add(0);
                screen.Add(row);
            }


            //replace the last pixel temporarily as a test to make sure the screen stuff works
            replacePixel(64, 0, 1);
            drawScreen();

            Stopwatch timer = new Stopwatch();
            timer.Start();
            while (timer.Elapsed < TimeSpan.FromSeconds(1)) ;

            for (int i = 0; i < programContent.Length; i++)
            {
                currentOpcode = programContent[i] + "" + programContent[i + 1];
                Console.WriteLine("Current opcode: " + currentOpcode);
                OpCodes.Add(currentOpcode);
                //increment i to account for the extra byte
                i += 1;
            }

            for (int i = 0; i < OpCodes.Count; i++)
                decodeOpCode(OpCodes[i], i);

            for (int i = 0; i < devOutputs.Count; i++)
                Console.WriteLine(devOutputs[i]);
        }

        public static void decodeOpCode(string opcode, int opcodeIndex)
        {
            switch (opcode[0])
            {
                case 'E':
                    if (opcode[1] == '0')
                    {
                        Console.Clear();
                        devOutputs.Add("Clearing screen...");
                        pc += 2;
                    }
                    break;
                case '1':
                    int[] hexDigits = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
                    int hexDigit = Convert.ToInt32(opcode[1].ToString(), 16);

                    if (hexDigits.Contains(hexDigit))
                    {
                        pc += 2;
                        int hexDigit1 = Convert.ToInt32(OpCodes[opcodeIndex + 1][0].ToString(), 16);
                        int hexDigit2 = Convert.ToInt32(OpCodes[opcodeIndex + 1][1].ToString(), 16);

                        if (hexDigits.Contains(hexDigit1) && hexDigits.Contains(hexDigit2))
                        {
                            devOutputs.Add("Setting PC to " + opcode[1] + OpCodes[opcodeIndex + 1]);
                            pc = Convert.ToInt32(opcode[1] + OpCodes[opcodeIndex + 1], 16);
                            pc += 2;
                        }
                    }
                    break;
            }
        }

        public static void drawScreen()
        {
            Console.Clear();
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
            screen[py][px] = status;
        }
    }
}