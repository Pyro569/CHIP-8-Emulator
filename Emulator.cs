using System.Diagnostics;

namespace Chip8
{
    class Emulator
    {
        static string program = "";
        static string programContent = "";
        static string currentOpcode;

        static int targettedFPS = 60;
        public static bool emulating;


        public static void Main(string[] args)
        {
            CPU cpu = new CPU();
            SystemScreen screen = new SystemScreen();
            emulating = true;

            //get the binary input as an argument and read the contents of the binary
            program = args[0];
            StreamReader sr = new StreamReader(program);
            programContent = sr.ReadToEnd();

            //hexdump the binary
            byte[] bytes = System.Text.Encoding.UTF8.GetBytes(programContent);
            programContent = Convert.ToHexString(bytes);

            //replace the last pixel temporarily as a test to make sure the screen stuff works
            screen.initScreen();
            screen.replacePixel(64, 0, 1);
            screen.drawScreen();

            Stopwatch timer = new Stopwatch();
            timer.Start();
            while (timer.Elapsed < TimeSpan.FromSeconds(1)) ;

            for (int i = 0; i < programContent.Length; i++)
            {
                currentOpcode = programContent[i] + "" + programContent[i + 1];
                Console.WriteLine("Current opcode: " + currentOpcode);
                cpu.OpCodes.Add(currentOpcode);
                //increment i to account for the extra byte
                i += 1;
            }

            for (int i = 0; i < cpu.OpCodes.Count; i++)
                cpu.decodeOpCode(cpu.OpCodes[i], i, cpu);

            for (int i = 0; i < cpu.devOutputs.Count; i++)
                Console.WriteLine(cpu.devOutputs[i]);
        }
    }
}