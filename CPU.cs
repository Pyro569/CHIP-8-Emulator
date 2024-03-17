namespace Chip8
{
    class CPU
    {
        public byte[] memory = new byte[0x1000];
        public ushort[] stack = new ushort[16];
        public byte[] v = new byte[16];
        public ushort _i = 0;
        int pc = 0;

        public int[] hexDigits = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        public List<string> devOutputs = new List<string>();
        public List<string> OpCodes = new List<string>();

        public void decodeOpCode(string opcode, int opcodeIndex, CPU cpu)
        {
            switch (opcode[0])
            {
                case 'E':
                    //00E0
                    if (opcode[1] == '0')
                    {
                        Console.Clear();
                        devOutputs.Add("Clearing screen...");
                        pc += 2;
                    }
                    break;
                case '1':
                    int hexDigit = Convert.ToInt32(opcode[1].ToString(), 16);

                    if (hexDigits.Contains(hexDigit))
                    {
                        pc += 2;
                        int hexDigit1 = Convert.ToInt32(OpCodes[opcodeIndex + 1][0].ToString(), 16);
                        int hexDigit2 = Convert.ToInt32(OpCodes[opcodeIndex + 1][1].ToString(), 16);

                        //1NNN
                        if (hexDigits.Contains(hexDigit1) && hexDigits.Contains(hexDigit2))
                        {
                            devOutputs.Add("Setting PC to " + opcode[1] + OpCodes[opcodeIndex + 1]);
                            pc = Convert.ToInt32(opcode[1] + OpCodes[opcodeIndex + 1], 16);
                            pc += 2;
                        }
                    }
                    break;
                case '6':
                    if (hexDigits.Contains(Convert.ToInt32(opcode[1].ToString(), 16)))
                    {
                        pc += 2;
                        int hexDigit1 = Convert.ToInt32(OpCodes[opcodeIndex + 1][0].ToString(), 16);
                        int hexDigit2 = Convert.ToInt32(OpCodes[opcodeIndex + 1][1].ToString(), 16);

                        //6XNN
                        if (hexDigits.Contains(hexDigit1) && hexDigits.Contains(hexDigit2))
                        {
                            v[Convert.ToInt32(opcode[1].ToString(), 16)] = Convert.ToByte(Convert.ToInt32(OpCodes[opcodeIndex + 1].ToString(), 16));
                            devOutputs.Add("Setting V" + opcode[1].ToString() + " to " + Convert.ToInt32(OpCodes[opcodeIndex + 1].ToString(), 16));
                        }
                    }
                    break;
                case '7':
                    break;
            }
        }
    }
}