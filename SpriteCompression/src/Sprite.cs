using System;
using System.IO;
using System.Collections.Generic;
using System.Drawing;

namespace SpriteCompression
{
    public class Sprite
    {
        public int Width { get; private set; }
        public int Height { get; private set; }
        public int[] Pixels { get; private set; }

        public Sprite(string path)
        {
            Bitmap bitmap = new Bitmap(path);
            Width = bitmap.Width;
            Height = bitmap.Height;
            Pixels = new int[Width * Height];
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    Color pixel = bitmap.GetPixel(x, y);
                    Pixels[x + y * Width] = pixel.A << 24 | pixel.R << 16 | pixel.B << 8 | pixel.B;
                }
            }
        }

        public Sprite(BinaryReader br)
        {
            Width = 400;
            Height = 400;
            Pixels = new int[Width * Height];
            for (int i = 0; i < 400 * 400; i++)
                Pixels[i] = br.ReadInt32();
        }
    }
}
