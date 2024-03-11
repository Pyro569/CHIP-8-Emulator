namespace Chip8
{
    class Emulator
    {
        static string program = "";
        static int pc = 0;
        static string programContent = "";
        static string currentOpcode;

        public static void Main(string[] args)
        {
            CPU cpu = new CPU();
            //get the binary input as an argument and read the contents of the binary
            program = args[0];
            StreamReader sr = new StreamReader(program);
            programContent = sr.ReadToEnd();

            //hexdump the binary
            byte[] bytes = System.Text.Encoding.UTF8.GetBytes(programContent);
            programContent = Convert.ToHexString(bytes);

            for (int i = 0; i < programContent.Length; i++)
            {
                //for some reason if i do this in one move it all ends up as numbers and i dont understand why
                currentOpcode = programContent[i] + "";
                currentOpcode += programContent[i + 1];
                //increment i to account for the extra pc counter & increment pc
                i += 1;
                pc += 2;

                decodeOpCode(currentOpcode);
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
    }
}