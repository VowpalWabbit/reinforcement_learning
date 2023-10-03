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

        public long ElapsedMicroseconds { get; private set; }

        public List<long> ElapsedMicrosecondsPerExample { get; } = new List<long>();

        public static Statistics operator + (Statistics s1, Statistics s2)
        {
            return new Statistics()
            {
                Bytes = s1.Bytes + s2.Bytes,
                Messages = s1.Messages + s2.Messages,
                ElapsedMicroseconds = Math.Max(s1.ElapsedMicroseconds, s2.ElapsedMicroseconds)
            };
        }

        public void Update(double byteCount)
        {
            Bytes += byteCount;
            Messages++;
            long currentElapsedMicroseconds = Timer.ElapsedTicks * 1000000L / Stopwatch.Frequency;
            ElapsedMicrosecondsPerExample.Add(currentElapsedMicroseconds - ElapsedMicroseconds);
            ElapsedMicroseconds = currentElapsedMicroseconds;
        }

        public void Print()
        {
            double elapsedSeconds = ElapsedMicroseconds / 1_000_000.0;
            double mbSent = Bytes / (1024 * 1024);
            double msgsPerSecond = Messages / elapsedSeconds;

            Console.WriteLine($"Data sent: {mbSent} MB");
            Console.WriteLine($"Time taken: {elapsedSeconds} secs");
            Console.WriteLine($"Throughput: {mbSent / elapsedSeconds} MB/s");
            Console.WriteLine($"Messages sent: {Messages}");
            Console.WriteLine($"Avg Message size: {Bytes / (1024 * Messages)} KB");
            Console.WriteLine($"Msg/s: {msgsPerSecond}");
            Console.WriteLine($"Latency: {elapsedSeconds / Messages * 1_000_000} μs");

            // Print average latency of the top 1% most microsecond examples
            int top1PercentCount = (int)(0.01 * ElapsedMicrosecondsPerExample.Count);
            if (top1PercentCount > 0)
            {
                List<long> sortedLatencies = new List<long>(ElapsedMicrosecondsPerExample);
                sortedLatencies.Sort((a, b) => b.CompareTo(a));  // sort in descending order
                long totalTop1PercentLatency = 0;
                for (int i = 0; i < top1PercentCount; i++)
                {
                    totalTop1PercentLatency += sortedLatencies[i];
                }
                double averageTop1PercentLatency = (double)totalTop1PercentLatency / top1PercentCount;
                Console.WriteLine($"Average latency of top 1% examples: {averageTop1PercentLatency} μs");
            }
        }
    }
}
