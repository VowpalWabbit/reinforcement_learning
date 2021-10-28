#!/usr/bin/env python

from log_gen import *
import data
from pathlib import Path
from sympy.combinatorics import Permutation

class Env:
    root = Path('../../external_parser/unit_tests/test_files/')
    valid_joined_logs = root.joinpath('valid_joined_logs')

    def cb_pdrop_05(self):
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""

        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_05.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([
                data.JoinedEvent(
                    event=data.CbEvent(id = 'id', context=ctx1, actions=[2,1], pass_prob=0.5)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id= 'id', value=1))])

    def cb_pdrop_1(self):       
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2 = """{"S": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_1.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([
                data.JoinedEvent(
                    event=data.CbEvent(id = 'id1', context=ctx1, actions=[2,1], pass_prob=0)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id = 'id1', value=1))])
            with_pdrop.write_regular_message([
                data.JoinedEvent(
                    event=data.CbEvent(id = 'id2', context=ctx2, actions=[2,1], pass_prob=1)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id = 'id2', value=1))])

    def multistep_2_episodes(self):
        ctx = """{{"{0}": {{"f": 1}}, "_multi":[{{"a1":"f1"}},{{"a2":"f2"}}]}}"""

        with BinLogWriter(self.valid_joined_logs.joinpath('multistep_2_episodes.fb')) as writer:
            writer.write_file_magic()
            writer.write_checkpoint_info()
            writer.write_regular_message([
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep1', event_id='1', context=ctx.format('A'))),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep1', event_id='2', previous_id='1', context=ctx.format('B'))),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep1', secondary_id='1', value = 2))])
            writer.write_regular_message([
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='1', context=ctx.format('C'))),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='2', previous_id='1', context=ctx.format('D'))),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', value = 3))])

    def multistep_3_deferred_episodes(self):
        ctx = """{{"{0}": {{"f": 1}}, "_multi":[{{"a1":"f1"}},{{"a2":"f2"}}]}}"""

        with BinLogWriter(self.valid_joined_logs.joinpath('multistep_3_deferred_episodes.fb')) as writer:
            writer.write_file_magic()
            writer.write_checkpoint_info()
            writer.write_regular_message([
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep1', event_id='1', context=ctx.format('A'), deferred=True)),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep1', event_id='2', previous_id='1', context=ctx.format('B'), deferred=True)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep1', secondary_id='1', value = 2)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep1', secondary_id='1'))])               
            writer.write_regular_message([
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='1', context=ctx.format('C'), deferred=True)),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='2', previous_id='1', context=ctx.format('D'), deferred=True)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', value = 3)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2'))])
            writer.write_regular_message([
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep3', event_id='1', context=ctx.format('E'), deferred=True)),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep3', event_id='2', previous_id='1', context=ctx.format('F'), deferred=True)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep3', value = 4))])

    def multistep_unordered_episodes(self):
        ctx = """{{"{0}": {{"f": 1}}, "_multi":[{{"a1":"f1"}},{{"a2":"f2"}}]}}"""

        dt_before = datetime(2021, 1, 1, 0, 0, 0)
        dt_after = datetime(2021, 1, 1, 1, 0, 0)

        with BinLogWriter(self.valid_joined_logs.joinpath('multistep_unordered_episodes.fb')) as writer:
            writer.write_file_magic()
            writer.write_checkpoint_info()
            # 1(A) -> 2(B)

            msg = [
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep1', event_id='1', context=ctx.format('A'))),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep1', event_id='2', previous_id='1', context=ctx.format('B'))),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep1', secondary_id='1', value = 1)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep1', secondary_id='2', value = 2))]

            perm = Permutation([[0, 1], [2, 3]])
            writer.write_regular_message(perm(msg)) 

            #    1(C)   <   4(F)
            #   /          /    \
            #  2(D)       5(G) < 6(H)
            #   \
            #    3(E)      
            # 
            msg = [
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='1', context=ctx.format('C')),
                    timestamp=dt_before), 
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='2', previous_id='1', context=ctx.format('D')),
                    timestamp=dt_after),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='3', previous_id='2', context=ctx.format('E')),
                    timestamp=dt_before),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='4', context=ctx.format('F')),
                    timestamp=dt_after),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='5', previous_id='4', context=ctx.format('G')),
                    timestamp=dt_before),
                data.JoinedEvent(
                    event=data.MultiStepEvent(episode_id='ep2', event_id='6', previous_id='4', context=ctx.format('H')),
                    timestamp=dt_after),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', secondary_id='1', value = 1)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', secondary_id='2', value = 2)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', secondary_id='3', value = 3)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', secondary_id='4', value = 4)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', secondary_id='5', value = 5)),
                data.JoinedEvent(
                    event=data.OutcomeEvent(primary_id='ep2', secondary_id='6', value = 6))]
            perm = Permutation([[0, 5], [1, 3], [2, 6, 7, 8, 9, 10, 11], [4]])
            writer.write_regular_message(perm(msg))  
                
def main():
    env = Env()
    env.cb_pdrop_05()
    env.cb_pdrop_1()
    env.multistep_2_episodes()
    env.multistep_3_deferred_episodes()
    env.multistep_unordered_episodes()

if __name__ == "__main__":
    main()
