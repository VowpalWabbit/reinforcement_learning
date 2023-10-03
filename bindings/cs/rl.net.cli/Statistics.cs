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
        private int lastGCCount;
        public List<bool> IsGCDistorted { get; } = new List<bool>();

        public static Statistics operator +(Statistics s1, Statistics s2)
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

            // Check if a GC occurred since the last update
            int currentGCCount = GC.CollectionCount(0);
            IsGCDistorted.Add(currentGCCount > lastGCCount);
            lastGCCount = currentGCCount;

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

            // Compute average latency of the top 1% with GC distortions
            List<long> allLatencies = new List<long>(ElapsedMicrosecondsPerExample);
            allLatencies.Sort((a, b) => b.CompareTo(a));
            int top1PercentCount = (int)(0.01 * allLatencies.Count);
            long totalTop1PercentLatency = 0;
            for (int i = 0; i < top1PercentCount; i++)
            {
                totalTop1PercentLatency += allLatencies[i];
            }
            double averageTop1PercentLatency = (double)totalTop1PercentLatency / top1PercentCount;
            Console.WriteLine($"Average latency of top 1% examples (with GC distortions): {averageTop1PercentLatency} μs");

            // Compute average latency of the top 1% without GC distortions
            List<long> filteredLatencies = new List<long>();
            for (int i = 0; i < ElapsedMicrosecondsPerExample.Count; i++)
            {
                if (!IsGCDistorted[i])
                {
                    filteredLatencies.Add(ElapsedMicrosecondsPerExample[i]);
                }
            }
            filteredLatencies.Sort((a, b) => b.CompareTo(a));
            top1PercentCount = (int)(0.01 * filteredLatencies.Count);
            totalTop1PercentLatency = 0;
            for (int i = 0; i < top1PercentCount; i++)
            {
                totalTop1PercentLatency += filteredLatencies[i];
            }
            averageTop1PercentLatency = (double)totalTop1PercentLatency / top1PercentCount;
            Console.WriteLine($"Average latency of top 1% examples (without GC distortions): {averageTop1PercentLatency} μs");
        }
    }
}
