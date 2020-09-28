using System;
using System.Collections.Generic;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    internal class ItemStats<TAction>
    {
        public Dictionary<TAction, int[]> ActionCounts
        {
            get;
            private set;
        } = new Dictionary<TAction, int[]>();

        public int TotalActions
        {
            get;
            private set;
        } = 0;

        public void IncrementAction(TAction action, float outcome)
        {
            if (!this.ActionCounts.ContainsKey(action))
            {
                this.ActionCounts.Add(action, new int[2]);
            }

            if (outcome > 0.00001f)
            {
                this.ActionCounts[action][0]++;
            }

            this.ActionCounts[action][1]++;
            this.TotalActions++;
        }

        public string GetSummary(TAction action)
        {        
            int wins = this.ActionCounts[action][0];
            int total = this.ActionCounts[action][1];

            return $"{action}: Out of {total} plays, {wins} wins. Item saw {this.TotalActions} decisions.";
        }
    }

    internal class StatisticsCalculator<TItem, TAction>
    {
        private Dictionary<TItem, ItemStats<TAction>> itemToStatsMap;

        public int TotalActions
        {
            get;
            private set;
        } = 0;

        public StatisticsCalculator()
        {
            this.itemToStatsMap = new Dictionary<TItem, ItemStats<TAction>>();
        }

        private ItemStats<TAction> EnsureItemStats(TItem item)
        {
            ItemStats<TAction> result;
            if (!this.itemToStatsMap.TryGetValue(item, out result))
            {
                this.itemToStatsMap[item] = result = new ItemStats<TAction>();
            }

            return result;
        }

        public void Record(TItem item, TAction chosenAction, float outcome)
        {
            this.EnsureItemStats(item).IncrementAction(chosenAction, outcome);
        }


        public string GetStats(TItem item, TAction chosenAction)
        {
            return this.EnsureItemStats(item).GetSummary(chosenAction);
        }
    }
}