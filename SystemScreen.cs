namespace Chip8
{
    class SystemScreen
    {
        public static List<List<int>> screenArray = new List<List<int>>();

        public void initScreen()
        {
            for (int i = 0; i < 33; i++)
            {
                List<int> row = new List<int>();
                for (int k = 0; k < 65; k++)
                    row.Add(0);
                screenArray.Add(row);
            }
        }

        public void drawScreen()
        {
            Console.Clear();
            for (int i = 0; i < screenArray.Count; i++)
            {
                for (int k = 0; k < screenArray[i].Count; k++)
                    if (screenArray[i][k] == 1)
                        Console.Write("■");
                    else
                        Console.Write(" ");
                Console.Write("\n");
            }
        }

        public void replacePixel(int px, int py, int status)
        {
            screenArray[py][px] = status;
        }
    }
}