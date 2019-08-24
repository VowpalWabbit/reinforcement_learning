using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Rl.Net.Cli
{
    public class Statistics
    {
        public int Messages { get; private set; }

        public double Bytes { get; private set; }

        public Stopwatch Timer { get; } = Stopwatch.StartNew();

        public long ElapsedMs { get; private set; }

        public static Statistics operator + (Statistics s1, Statistics s2)
        {
            return new Statistics()
            {
                Bytes = s1.Bytes + s2.Bytes,
                Messages = s1.Messages + s2.Messages,
                ElapsedMs = Math.Max(s1.ElapsedMs, s2.ElapsedMs)
            };
        }

        public void Update(double byteCount)
        {
            Bytes += byteCount;
            Messages++;
            ElapsedMs = Timer.ElapsedMilliseconds;
        }

        public void Print()
        {
            Console.WriteLine($"Data sent: {this.Bytes / (1024 * 1024)} MB");
            Console.WriteLine($"Time taken: {(this.ElapsedMs / 1000)} secs");
            Console.WriteLine($"Throughput: {this.Bytes / ((1024 * 1024) * this.ElapsedMs / 1000)} MB / s");
            Console.WriteLine($"Messages sent: {this.Messages}");
            Console.WriteLine($"Avg Message size: {this.Bytes / (1024 * this.Messages)}");
            Console.WriteLine($"Msg/s: {this.Messages / (this.ElapsedMs / 1000)}");
        }
    }
}
