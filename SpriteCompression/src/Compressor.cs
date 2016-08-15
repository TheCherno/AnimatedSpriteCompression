using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SpriteCompression
{
    public class Compressor
    {
        public readonly byte QUALITY; // 0-5, 0 is lossless, 5 is worst
        private int m_SimilarityThreshold;
        private byte m_CompressionMode = 0; // 0-1, 0 is uncompressed, 1 is default gzip

        private Sprite[] m_Sprites;
        private int m_WindowSize;

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct Header
        {
            public byte h0, h1;             // 08 08
            public byte quality;            // 0-5
            public byte format;             // 0 = ARGB
            public byte r0, r1;             // Reserved
            public ushort frames;           // number of frames
            public ushort width, height;    // Size of animation
        }

        public Compressor(Sprite[] sprites, byte quality, int windowSize = 16)
        {
            m_Sprites = sprites;
            QUALITY = quality;
            m_SimilarityThreshold = 10 * quality;
            m_WindowSize = windowSize;
        }

        public byte[] Compress()
        {
            Console.WriteLine("Compressing {0} sprites...", m_Sprites.Length);
            Console.WriteLine("\tQuality={0}", QUALITY);
            Console.WriteLine("\tSimilarityThreshold={0}", m_SimilarityThreshold);
            Console.WriteLine("\tCompressionMode={0}", m_CompressionMode);
            Console.WriteLine();

            MemoryStream stream = new MemoryStream();
            BinaryWriter writer = new BinaryWriter(stream);

            long count = WriteHeader(writer);
            count = WriteKeyframe(writer, m_Sprites[0]);

            Console.WriteLine("Compressed key frame {0} - {1} bytes", 0, count);
            int[] pixelData = new int[m_Sprites[0].Pixels.Length];
            m_Sprites[0].Pixels.CopyTo(pixelData, 0);

            for (int i = 1; i < m_Sprites.Length; i++)
            {
                count = WriteDeltaFrame3(writer, m_Sprites[i], m_Sprites[i - 1].Pixels);
                Console.WriteLine("Compressed frame {0} - {1} bytes", i, count);
            }
            return stream.ToArray();
        }

        private long WriteHeader(BinaryWriter writer)
        {
            long pos = writer.BaseStream.Position;

            Header header;
            header.h0 = 0x08;
            header.h1 = 0x08;
            header.quality = QUALITY;
            header.format = 0;
            header.r0 = header.r1 = 0;
            header.frames = (ushort)m_Sprites.Length;
            header.width = (ushort)m_Sprites[0].Width;
            header.height = (ushort)m_Sprites[0].Height;

            writer.Write(StructToBytes(header));
            return writer.BaseStream.Position - pos;
        }

        private byte[] Zip(byte[] v)
        {
            MemoryStream result = new MemoryStream();
            using (System.IO.Compression.GZipStream gz = new System.IO.Compression.GZipStream(result, System.IO.Compression.CompressionLevel.Optimal))
                gz.Write(v, 0, v.Length);
            return result.ToArray();
        }

        public long WriteKeyframe(BinaryWriter writer, Sprite frame)
        {
            long pos = writer.BaseStream.Position;

            writer.Write((byte)0); // 0 = keyframe
            writer.Write(m_CompressionMode); // TODO: Could be global for all frames

            MemoryStream stream = new MemoryStream(frame.Width * frame.Height * 4); // 4 bpc (ARGB)
            for (int y = 0; y < frame.Height; y++)
            {
                for (int x = 0; x < frame.Width; x++)
                {
                    int pixel = frame.Pixels[x + y * frame.Width];
                    stream.Write(BitConverter.GetBytes(pixel), 0, 4);
                }
            }

            byte[] buffer = stream.ToArray();
            if (m_CompressionMode > 0)
                buffer = Zip(buffer);

            writer.Write(buffer);
            return writer.BaseStream.Position - pos;
        }

        public long WriteDeltaFrame(BinaryWriter writer, Sprite frame, int[] previousBuffer)
        {
            long position = writer.BaseStream.Position;

            writer.Write((byte)1); // 1 = delta frame
            writer.Write(m_CompressionMode); // TODO: Could be global for all frames

            MemoryStream stream = new MemoryStream(frame.Pixels.Length * 4); // 4 bpc (ARGB)
            MemoryStream temp = new MemoryStream();
            int runCount = 0;
            for (int y = 0; y < frame.Height; y++)
            {
                for (int x = 0; x < frame.Width; x++)
                {
                    int pp = previousBuffer[x + y * frame.Width];
                    int cp = frame.Pixels[x + y * frame.Width];
                    if (pp == cp || ChannelDiff(pp, cp) < m_SimilarityThreshold)
                    {
                        if (temp.Length > 0)
                        {
                            var tempBuffer = temp.ToArray();
                            temp = new MemoryStream();
                            stream.Write(BitConverter.GetBytes((ushort)((tempBuffer.Length / 4) | 0x8000)), 0, 2);
                            stream.Write(tempBuffer, 0, tempBuffer.Length);
                        }
                        runCount++;
                    }
                    else
                    {
                        if (runCount > 0)
                        {
                            stream.Write(BitConverter.GetBytes((short)runCount), 0, 2);
                            runCount = 0;
                        }
                        temp.Write(BitConverter.GetBytes(cp), 0, 4);
                    }

                    if (runCount == 0x7fff)
                    {
                        stream.Write(BitConverter.GetBytes((short)runCount), 0, 2);
                        runCount = 0;
                    }
                }
            }
            if (temp.Length > 0)
            {
                var tempBuffer = temp.ToArray();
                temp = new MemoryStream();
                stream.Write(BitConverter.GetBytes((ushort)((tempBuffer.Length / 4) | 0x8000)), 0, 2);
                stream.Write(tempBuffer, 0, tempBuffer.Length);
            }
            if (runCount > 0)
            {
                stream.Write(BitConverter.GetBytes((short)runCount), 0, 2);
                runCount = 0;
            }

            byte[] buffer = stream.ToArray();
            if (m_CompressionMode > 0)
                buffer = Zip(buffer);

            writer.Write(buffer);
            return writer.BaseStream.Position - position;
        }

        public long WriteDeltaFrame2(BinaryWriter writer, Sprite frame, int[] previousBuffer)
        {
            long position = writer.BaseStream.Position;

            writer.Write((byte)1); // 1 = delta frame
            writer.Write(m_CompressionMode); // TODO: Could be global for all frames

            MemoryStream stream = new MemoryStream(frame.Pixels.Length * 4); // 4 bpc (ARGB)
            MemoryStream temp = new MemoryStream();
            int runCount = 0;
            for (int y = 0; y < frame.Height; y++)
            {
                for (int x = 0; x < frame.Width; x++)
                {
                    int pp = previousBuffer[x + y * frame.Width];
                    int cp = frame.Pixels[x + y * frame.Width];

                    if (pp == cp || ChannelDiff(pp, cp) < m_SimilarityThreshold)
                    {
                        if (temp.Length > 0)
                        {
                            stream.Write(BitConverter.GetBytes((ushort)runCount), 0, 2);
                            runCount = 0;

                            var tempBuffer = temp.ToArray();
                            temp = new MemoryStream();
                            stream.Write(BitConverter.GetBytes((ushort)(tempBuffer.Length / 4)), 0, 2);
                            stream.Write(tempBuffer, 0, tempBuffer.Length);
                        }
                        runCount++;
                    }
                    else
                    {
                        temp.Write(BitConverter.GetBytes(cp), 0, 4);
                    }

                    if (runCount == 0xffff)
                    {
                        stream.Write(BitConverter.GetBytes((ushort)runCount), 0, 2);
                        stream.Write(BitConverter.GetBytes((ushort)0), 0, 2);
                        runCount = 0;
                    }
                }
            }
            if (temp.Length > 0)
            {
                stream.Write(BitConverter.GetBytes((ushort)runCount), 0, 2);

                var tempBuffer = temp.ToArray();
                temp = new MemoryStream();
                stream.Write(BitConverter.GetBytes((ushort)(tempBuffer.Length / 4)), 0, 2);
                stream.Write(tempBuffer, 0, tempBuffer.Length);
            }
            else if (runCount > 0)
            {
                stream.Write(BitConverter.GetBytes((ushort)runCount), 0, 2);
                stream.Write(BitConverter.GetBytes((ushort)0), 0, 2);
            }

            byte[] buffer = stream.ToArray();
            if (m_CompressionMode > 0)
                buffer = Zip(buffer);

            writer.Write(buffer);
            return writer.BaseStream.Position - position;
        }

        public long WriteDeltaFrame3(BinaryWriter writer, Sprite frame, int[] previousBuffer)
        {
            long position = writer.BaseStream.Position;

            writer.Write((byte)1); // 1 = delta frame
            writer.Write(m_CompressionMode); // TODO: Could be global for all frames

            MemoryStream stream = new MemoryStream(frame.Pixels.Length * 4); // 4 bpc (ARGB)
            MemoryStream temp = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(stream);
            int pixelCount = frame.Width * frame.Height;

            int skipped = 0;
            int copied = 0;
            bool emit = false;
            for (int i = 0; i < pixelCount; i++)
            {
                int pp = previousBuffer[i];
                int cp = frame.Pixels[i];

                if (copied == 0xffff || skipped == 0xffff)
                {
                    EmitPacket(bw, frame.Pixels, i, skipped, copied);
                    emit = false;
                    skipped = 0;
                    copied = 0;
                }

                if (pp == cp || ChannelDiff(pp, cp) < m_SimilarityThreshold)
                {
                    if (emit)
                    {
                        EmitPacket(bw, frame.Pixels, i, skipped, copied);
                        emit = false;
                        skipped = 0;
                        copied = 0;
                    }
                    skipped++;
                }
                else
                {
                    copied++;
                    if (skipped < m_WindowSize)
                    {
                        copied += skipped;
                        skipped = 0;
                    }
                    else
                        emit = true;
                }
            }
            if (skipped > 0 || copied > 0)
                EmitPacket(bw, frame.Pixels, pixelCount, skipped, copied);

            byte[] buffer = stream.ToArray();
            if (m_CompressionMode > 0)
                buffer = Zip(buffer);

            writer.Write(buffer);
            return writer.BaseStream.Position - position;
        }

        private void EmitPacket(BinaryWriter stream, int[] pixels, int cursor, int skipped, int copied)
        {
            MemoryStream pixelsToCopy = new MemoryStream();
            for (int i = 0; i < copied; i++)
            {
                var pixel = BitConverter.GetBytes(pixels[cursor - copied + i]);
                pixelsToCopy.Write(pixel, 0, pixel.Length);
            }
            stream.Write((ushort)skipped);
            stream.Write((ushort)copied);

            var copiedPixels = pixelsToCopy.ToArray();
            stream.Write(copiedPixels, 0, copiedPixels.Length);
        }

        public int ChannelDiff(int c0, int c1)
        {
            byte[] a = BitConverter.GetBytes(c0);
            byte[] b = BitConverter.GetBytes(c1);

            int result = 0;
            for (int i = 0; i < 4; i++)
                result = Math.Max(result, Math.Abs(a[i] - b[i]));
            return result;
        }

        private byte[] StructToBytes(object str)
        {
            int objsize = Marshal.SizeOf(typeof(Header));
            byte[] result = new byte[objsize];
            IntPtr buff = Marshal.AllocHGlobal(objsize);
            Marshal.StructureToPtr(str, buff, true);
            Marshal.Copy(buff, result, 0, objsize);
            Marshal.FreeHGlobal(buff);
            return result;
        }


    }
}
