#!/usr/bin/env python

from log_gen import *
import data
from pathlib import Path

class Env:
    root = Path('../../external_parser/unit_tests/test_files/')
    valid_joined_logs = root.joinpath('valid_joined_logs')

    def cb_pdrop_05(self):
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""

        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_05.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([
                data.CbEvent(id = 'id', context=ctx1, actions=[2,1], pass_prob=0.5),
                data.OutcomeEvent(primary_id= 'id', value=1)])

    def cb_pdrop_1(self):       
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2 = """{"S": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_1.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([
                data.CbEvent(id = 'id1', context=ctx1, actions=[2,1], pass_prob=0),
                data.OutcomeEvent(primary_id = 'id1', value=1)])
            with_pdrop.write_regular_message([
                data.CbEvent(id = 'id2', context=ctx2, actions=[2,1], pass_prob=1),
                data.OutcomeEvent(primary_id = 'id2', value=1)])

    def multistep_2_episodes(self):
        ctx1_1 = """{"A": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx1_2 = """{"B": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2_1 = """{"C": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2_2 = """{"D": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('multistep_2_episodes.fb')) as writer:
            writer.write_file_magic()
            writer.write_checkpoint_info()
            writer.write_regular_message([
                data.MultiStepEvent(episode_id='ep1', event_id='1', context=ctx1_1),
                data.MultiStepEvent(episode_id='ep1', event_id='2', previous_id='1', context=ctx1_2),
                data.OutcomeEvent(primary_id='ep1', secondary_id='1', value = 2)])
            writer.write_regular_message([
                data.MultiStepEvent(episode_id='ep2', event_id='1', context=ctx2_1),
                data.MultiStepEvent(episode_id='ep2', event_id='2', previous_id='1', context=ctx2_2),
                data.OutcomeEvent(primary_id='ep2', value = 3)])

    def multistep_3_deferred_episodes(self):
        ctx1_1 = """{"A": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx1_2 = """{"B": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2_1 = """{"C": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2_2 = """{"D": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx3_1 = """{"E": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx3_2 = """{"F": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('multistep_3_deferred_episodes.fb')) as writer:
            writer.write_file_magic()
            writer.write_checkpoint_info()
            writer.write_regular_message([
                data.MultiStepEvent(episode_id='ep1', event_id='1', context=ctx1_1, deferred=True),
                data.MultiStepEvent(episode_id='ep1', event_id='2', previous_id='1', context=ctx1_2, deferred=True),
                data.OutcomeEvent(primary_id='ep1', secondary_id='1', value = 2),
                data.OutcomeEvent(primary_id='ep1', secondary_id='1')]),                
            writer.write_regular_message([
                data.MultiStepEvent(episode_id='ep2', event_id='1', context=ctx2_1, deferred=True),
                data.MultiStepEvent(episode_id='ep2', event_id='2', previous_id='1', context=ctx2_2, deferred=True),
                data.OutcomeEvent(primary_id='ep2', value = 3),
                data.OutcomeEvent(primary_id='ep2')]),  
            writer.write_regular_message([
                data.MultiStepEvent(episode_id='ep3', event_id='1', context=ctx3_1, deferred=True),
                data.MultiStepEvent(episode_id='ep3', event_id='2', previous_id='1', context=ctx3_2, deferred=True),
                data.OutcomeEvent(primary_id='ep3', value = 4)])
                

def main():
    env = Env()
    env.cb_pdrop_05()
    env.cb_pdrop_1()
    env.multistep_2_episodes()
    env.multistep_3_deferred_episodes()

if __name__ == "__main__":
    main()
