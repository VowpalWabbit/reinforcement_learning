using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Rl.Net.Cli
{
    public class OnlineWelford
    {
        int count;
        double mean, m2;
        double sum;
        double min = double.MaxValue;
        double max = double.MinValue;

        public void Update(double value)
        {
            sum += value;
            min = Math.Min(min, value);
            max = Math.Max(max, value);
            ++count;
            var d = value - mean;
            mean += d / count;
            var d2 = value - mean;
            m2 += d * d2;
        }

        public double EstimatedMean => mean;
        public double EstimatedVariance => m2 / count;
        public double SampleVariance => m2 / (count - 1);
        public double Min => min;
        public double Max => max;
        public int N => count;
        public double Sum => sum;
    }

    public interface IHistogram
    {
        void Update(double value);
        int Bins { get; }
        double Min { get; }
        double Max { get; }
        double ObservedMin { get; }
        double ObservedMax { get; }
        int[] Counts { get; }

    }
    public class Histogram : IHistogram
    {
        int[] counts;
        int count;
        double actualMin, actualMax;
        readonly double min, max;

        public Histogram(double min, double max, int bins)
        {
            if (max <= min + 1)
                max = min + 1;
            this.min = min;
            this.max = max;
            this.actualMin = double.MaxValue;
            this.actualMax = double.MinValue;
            this.counts = new int[bins];
        }

        public Histogram(OnlineWelford est, int bins)
        {
            //this should capture the majority of the data
            var range = 1.96 * Math.Sqrt(est.SampleVariance);
            min = Math.Max(0, est.EstimatedMean - range);
            max = est.EstimatedMean + range;
            if (max <= min + 1)
                max = min + 1;
            this.actualMin = double.MaxValue;
            this.actualMax = double.MinValue;
            this.counts = new int[bins];
        }


        public Histogram(JObject json)
        {
            min = (double)json["Min"];
            max = (double)json["Max"];
            actualMin = (double)json["ObservedMin"];
            actualMax = (double)json["ObservedMax"];

            int bins = (int)json["Bins"];
            this.counts = new int[bins];
            int idx = 0;
            foreach (var elem in json["Counts"])
            {
                this.counts[idx++] = (int)elem;
            }
            this.count = this.counts.Sum();
        }

        public double Sample(Random rand)
        {
            int draw = rand.Next(count);
            int d = draw;
            int idx = 0;
            for (int i = 0; i < counts.Length; ++i)
            {
                if (draw < counts[i])
                {
                    break;
                }
                draw -= counts[i];
                ++idx;
            }

            double step = (max - min) / (counts.Length - 1);
            double val = min + step * idx;
            return val;
        }

        public void Update(double val)
        {
            double ov = val;
            this.actualMin = Math.Min(this.actualMin, val);
            this.actualMax = Math.Max(this.actualMax, val);
            //clamp val min/max
            val = Math.Max(val, min);
            val = Math.Min(val, max);

            double step = (max - min) / (counts.Length - 1);
            int idx = Math.Max(0, Math.Min(counts.Length - 1, (int)Math.Floor((val - min) / step)));
            ++counts[idx];
            ++count;
        }

        //Ugg but ok
        public int Bins => this.counts.Length;
        public int[] Counts => counts;
        public double ObservedMin => actualMin;
        public double ObservedMax => actualMax;
        public double Min => min;
        public double Max => max;
    }

    class HistogramWithSampledBounds : IHistogram
    {
        readonly int samples;
        readonly int bins;
        List<double> values;
        Histogram histogram, tmpHistogram;

        public HistogramWithSampledBounds(int samples, int bins)
        {
            this.samples = samples;
            this.bins = bins;
            this.values = new List<double>(samples);
        }

        Histogram CreateHistogram()
        {
            if (values.Count == 0)
            {
                return new Histogram(0, 1, 1);
            }
            var res = new Histogram(values.Min(), values.Max(), bins);
            foreach (var v in values)
                res.Update(v);

            return res;
        }

        public void Update(double val)
        {
            tmpHistogram = null;
            if (histogram != null)
            {
                histogram.Update(val);
            }
            else
            {
                values.Add(val);
                if (values.Count >= samples)
                {
                    histogram = CreateHistogram();
                    values = null;
                }
            }
        }

        private Histogram Hist
        {
            get
            {
                if (histogram != null)
                    return histogram;
                if (tmpHistogram != null)
                    return tmpHistogram;
                tmpHistogram = CreateHistogram();
                return tmpHistogram;
            }
        }

        public int Bins => Hist.Bins;
        public int[] Counts => Hist.Counts;
        public double ObservedMin => Hist.ObservedMin;
        public double ObservedMax => Hist.ObservedMax;
        public double Min => Hist.Min;
        public double Max => Hist.Max;
    }


    public class HashedDistanceHistogram
    {
        int[] counts;
        int[] foundHashes;
        int insertHead;
        int notFound;
        int entries;
        HashSet<int> uniqueActions = new HashSet<int>();

        public HashedDistanceHistogram(int maxDistance)
        {
            counts = new int[maxDistance];
            foundHashes = new int[maxDistance];
        }

        public HashedDistanceHistogram(JObject json)
        {
            notFound = (int)json["MissCount"];
            entries = (int)json["Entries"];
            int bins = json["Hist"].Count();
            counts = new int[bins];
            foundHashes = new int[bins];
            int unique = (int)json["UniqueActions"];
            //silly hack
            this.uniqueActions = new HashSet<int>(Enumerable.Range(0, unique));

            int idx = 0;
            foreach (var elem in json["Hist"])
            {
                this.counts[idx++] = (int)elem;
            }

        }

        public int Entries => entries;
        public int MissCount => notFound;
        public int[] Hist => counts;
        public int UniqueActions => uniqueActions.Count;

        public void Update(string str)
        {
            int hash = str.GetHashCode();
            if (hash == 0)
                hash = ("_" + str).GetHashCode();
            uniqueActions.Add(hash);

            int distance = 0;
            int foundDistance = -1;
            for (int i = foundHashes.Length - 1; i >= 0; --i)
            {
                int idx = (i + insertHead) % foundHashes.Length;
                if (foundHashes[idx] == hash)
                {
                    foundDistance = distance;
                    break;
                }
                ++distance;
            }

            ++entries;

            if (foundDistance == -1)
            {
                ++notFound;
            }
            else
            {
                ++counts[foundDistance];
            }

            foundHashes[insertHead] = hash;
            insertHead = (insertHead + 1) % foundHashes.Length;
        }

        public override string ToString()
        {
            string hist = string.Join(' ', counts.Select(i => i.ToString()));
            string hash = string.Join(' ', foundHashes.Select(i => i.ToString()));

            return $"nf: {notFound} hist: {hist} hashes: {hash}";
        }

        public int Sample(Random rand)
        {
            int draw = rand.Next(entries);
            int d = draw;
            int idx = 0;
            for (int i = 0; i < counts.Length; ++i)
            {
                if (draw < counts[i])
                {
                    break;
                }
                draw -= counts[i];
                ++idx;
            }
            if (idx == counts.Length)
                idx = -1;
            return idx;
        }
    }
}
