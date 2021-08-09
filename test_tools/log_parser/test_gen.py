#!/usr/bin/env python

#%% setup

from log_gen import *
from pathlib import Path

class Env:
    root = Path('../../external_parser/unit_tests/test_files/')
    valid_joined_logs = root.joinpath('valid_joined_logs')

    def cb_pdrop_05(self):
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""

        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_05.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([mk_cb_payload(_ctx=ctx1, _actions=[2,1], _pdrop=0.5), mk_outcome()])

    def cb_pdrop_1(self):       
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2 = """{"S": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_1.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([mk_cb_payload(_ctx=ctx1, _actions=[2,1], _pdrop=1), mk_outcome()])
            with_pdrop.write_regular_message([mk_cb_payload(_ctx=ctx2, _actions=[2,1], _pdrop=0), mk_outcome()])

    def multistep_2_episodes(self):
        ctx1_1 = """{"A": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx1_2 = """{"B": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2_1 = """{"C": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2_2 = """{"D": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('multistep_2_episodes.fb')) as writer:
            writer.write_file_magic()
            writer.write_checkpoint_info()
            writer.write_regular_message([
                mk_multistep_payload(_episode_id='ep1', _event_id='1', _ctx=ctx1_1),
                mk_multistep_payload(_episode_id='ep1', _event_id='2', _previous_id='1', _ctx=ctx1_2),
                mk_outcome(_primary_id='ep1', _secondary_id='1', _value = 2)])
            writer.write_regular_message([
                mk_multistep_payload(_episode_id='ep2', _event_id='1', _ctx=ctx2_1),
                mk_multistep_payload(_episode_id='ep2', _event_id='2', _previous_id='1', _ctx=ctx2_2),
                mk_outcome(_primary_id='ep2', _value = 3)])

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
                mk_multistep_payload(_episode_id='ep1', _event_id='1', _ctx=ctx1_1, _deferred=True),
                mk_multistep_payload(_episode_id='ep1', _event_id='2', _previous_id='1', _ctx=ctx1_2, _deferred=True),
                mk_outcome(_primary_id='ep1', _secondary_id='1', _value = 2),
                mk_outcome(_primary_id='ep1', _secondary_id='1', _value = None)]),                
            writer.write_regular_message([
                mk_multistep_payload(_episode_id='ep2', _event_id='1', _ctx=ctx2_1, _deferred=True),
                mk_multistep_payload(_episode_id='ep2', _event_id='2', _previous_id='1', _ctx=ctx2_2, _deferred=True),
                mk_outcome(_primary_id='ep2', _value = 3),
                mk_outcome(_primary_id='ep2', _value = None)]),  
            writer.write_regular_message([
                mk_multistep_payload(_episode_id='ep3', _event_id='1', _ctx=ctx3_1, _deferred=True),
                mk_multistep_payload(_episode_id='ep3', _event_id='2', _previous_id='1', _ctx=ctx3_2, _deferred=True),
                mk_outcome(_primary_id='ep3', _value = 4)])
                


#%% Generate pdrop test data
def main():
    env = Env()
    env.cb_pdrop_05()
    env.cb_pdrop_1()
    env.multistep_2_episodes()
    env.multistep_3_deferred_episodes()

if __name__ == "__main__":
    main()
