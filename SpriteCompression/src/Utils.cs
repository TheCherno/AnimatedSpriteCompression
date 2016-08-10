using System;

namespace SpriteCompression
{
    public static class Utils
    {
        private static readonly string[] SizeSuffixes = { "bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };

        public static string SizeSuffix(long value)
        {
            if (value < 0)
                return "-" + SizeSuffix(-value);
            if (value == 0)
                return "0 bytes";

            int mag = (int)Math.Log(value, 1024);
            decimal adjustedSize = (decimal)value / (1L << (mag * 10));

            return string.Format("{0:n2} {1}", adjustedSize, SizeSuffixes[mag]);
        }

    }
}
