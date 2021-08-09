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
            with_pdrop.write_regular_message([mk_cb_payload(_ctx=ctx1, _actions=[2,1], _pdrop=0.5), mk_cb_reward()])

    def cb_pdrop_1(self):       
        ctx1 = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        ctx2 = """{"S": {"f": 1}, "_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
        with BinLogWriter(self.valid_joined_logs.joinpath('cb_joined_with_pdrop_1.fb')) as with_pdrop:
            with_pdrop.write_file_magic()
            with_pdrop.write_checkpoint_info()
            with_pdrop.write_regular_message([mk_cb_payload(_ctx=ctx1, _actions=[2,1], _pdrop=1), mk_cb_reward()])
            with_pdrop.write_regular_message([mk_cb_payload(_ctx=ctx2, _actions=[2,1], _pdrop=0), mk_cb_reward()])
            
#%% Generate pdrop test data
def main():
    env = Env()
    env.cb_pdrop_05()
    env.cb_pdrop_1()

if __name__ == "__main__":
    main()
