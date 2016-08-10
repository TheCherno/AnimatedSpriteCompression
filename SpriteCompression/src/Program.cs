using System;
using System.IO;

namespace SpriteCompression
{
    using static Utils;

    public class Program
    {
        public enum Mode { COMPRESS, DECOMPRESS }

        public Program(Mode mode, string input, string output = "", byte quality = 0)
        {
            if (mode == Mode.COMPRESS)
            {
                string[] spriteFiles = Directory.GetFiles(input);
                Sprite[] sprites = new Sprite[spriteFiles.Length];
                Console.Write("Reading {0} images...", sprites.Length);
                int i = 0;
                long size = 0;
                foreach (string file in spriteFiles)
                {
                    size += new FileInfo(file).Length;
                    sprites[i++] = new Sprite(file);
                }

                Console.WriteLine(" done.");

                Compressor compressor = new Compressor(sprites, quality);
                byte[] buffer = compressor.Compress();
                Console.WriteLine("Writing {0} bytes to {1}", buffer.Length, output);
                FileStream stream = File.Create(output);
                stream.Write(buffer, 0, buffer.Length);
                stream.Close();
                Console.WriteLine(" done.");
                Console.WriteLine("Compressed {0} to {1}.", SizeSuffix(size), SizeSuffix(buffer.Length));
            }
            else if (mode == Mode.DECOMPRESS)
            {
                Decompressor decompressor = new Decompressor(input);
                decompressor.Decompress();
            }
        }
       
        private static void PrintUsage()
        {
            Console.WriteLine("\tUsage: sc mode(compress|decompress) input-path output-path [quality]");
        }

        public static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                PrintUsage();
                return;
            }

            string output = "";
            if (args.Length > 2)
                output = args[2];

            byte quality = 0;
            if (args.Length > 3)
                quality = byte.Parse(args[3]);

            Mode mode = args[0] == "Decompress" ? Mode.DECOMPRESS : Mode.COMPRESS;
            new Program(mode, args[1], output, quality);
        }
    }
}
