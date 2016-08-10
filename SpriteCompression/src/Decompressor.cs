using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SpriteCompression
{
    public class Decompressor
    {

        private string m_InputFilePath;

        public Decompressor(string input)
        {
            m_InputFilePath = input;
        }

        public void Decompress()
        {
            MemoryStream ms = new MemoryStream(File.ReadAllBytes(m_InputFilePath));
            BinaryReader br = new BinaryReader(ms);
            br.ReadBytes(20);
            Sprite keyframe = DecodeKeyframe(br);
            WriteFrame(0, keyframe, keyframe.Pixels);
            ms.Close();
        }

        private Sprite DecodeKeyframe(BinaryReader br)
        {
            int size = br.ReadInt32();
            byte[] db = Unzip(br.ReadBytes(size));
            BinaryReader b = new BinaryReader(new MemoryStream(db));
            b.ReadInt32(); // ID
            return new Sprite(b);
        }

        private void DecodeDelta(int[] pixelData, byte[] deltaBuffer)
        {
            MemoryStream ms = new MemoryStream(deltaBuffer);
            BinaryReader br = new BinaryReader(ms);
            int pixelCount = pixelData.Length;
            int currentPixel = 0;
            br.ReadUInt16();
            while (pixelCount != currentPixel)
            {
                ushort nextStep = br.ReadUInt16();
                int count = nextStep & 0x7FFF;
                if ((nextStep & 0x8000) != 0)
                {
                    for (int i = 0; i < count; i++)
                        pixelData[currentPixel++] = br.ReadInt32();
                }
                else
                {
                    currentPixel += count;
                }
            }
        }

        private byte[] Unzip(byte[] buffer)
        {
            MemoryStream result = new MemoryStream();
            using (System.IO.Compression.GZipStream gz = new System.IO.Compression.GZipStream(new MemoryStream(buffer), System.IO.Compression.CompressionMode.Decompress))
            {
                byte[] b = new byte[4096];
                int count;
                while ((count = gz.Read(b, 0, b.Length)) > 0)
                    result.Write(buffer, 0, count);
            }
            return result.ToArray();
        }

        private void WriteFrame(int v, Sprite sprite, int[] pixels)
        {
            System.Drawing.Bitmap bmp = new System.Drawing.Bitmap(sprite.Width, sprite.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            var bmpData = bmp.LockBits(new System.Drawing.Rectangle(new System.Drawing.Point(0, 0), new System.Drawing.Size(sprite.Width, sprite.Height)), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            for (int y = 0; y < sprite.Height; y++)
            {
                System.Runtime.InteropServices.Marshal.Copy(pixels, sprite.Width * y, bmpData.Scan0 + bmpData.Stride * y, sprite.Width);
            }
            bmp.UnlockBits(bmpData);
            bmp.Save(string.Format("output-decode/Output-{0}.bmp", v), System.Drawing.Imaging.ImageFormat.Bmp);
            bmp.Dispose();
        }

    }
}
