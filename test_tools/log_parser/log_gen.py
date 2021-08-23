#! /usr/bin/env python3 -W ignore::DeprecationWarning

from reinforcement_learning.messages.flatbuff.v2.EventBatch import *
from reinforcement_learning.messages.flatbuff.v2.BatchMetadata import *
from reinforcement_learning.messages.flatbuff.v2.EventBatch import *
from reinforcement_learning.messages.flatbuff.v2.LearningModeType import LearningModeType

from reinforcement_learning.messages.flatbuff.v2.KeyValue import *
from reinforcement_learning.messages.flatbuff.v2.TimeStamp import *
from reinforcement_learning.messages.flatbuff.v2.FileHeader import *
from reinforcement_learning.messages.flatbuff.v2.JoinedEvent import *
from reinforcement_learning.messages.flatbuff.v2.JoinedPayload import *
from reinforcement_learning.messages.flatbuff.v2.CheckpointInfo import *
from reinforcement_learning.messages.flatbuff.v2.RewardFunctionType import *
from reinforcement_learning.messages.flatbuff.v2.ProblemType import *
from reinforcement_learning.messages.flatbuff.v2.EventEncoding import *

import flatbuffers
import sys
import struct
from datetime import datetime
import numpy as np

MSG_TYPE_HEADER = 0x55555555
MSG_TYPE_CHECKPOINT = 0x11111111
MSG_TYPE_REGULAR = 0xFFFFFFFF
MSG_TYPE_EOF = 0xAAAAAAAA

def EndVector(builder, size):
    from packaging import version
    if version.parse(flatbuffers.__version__).major >= 2:
        return builder.EndVector()
    else:
        return builder.EndVector(size)

def mk_timestamp(builder):
    time = datetime(2021, 1, 1, 0, 0, 0)
    return CreateTimeStamp(builder, time.year, time.month, time.day, time.hour, time.minute, time.second, int(time.microsecond))

def mk_offsets_vector(builder, arr, startFun):
    startFun(builder, len(arr))
    for i in reversed(range(len(arr))):
        builder.PrependUOffsetTRelative(arr[i])
    return EndVector(builder, len(arr))

def mk_bytes_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype='b'))



class BinLogWriter:
    def __init__(self, file_name):
        self.file = open(file_name, 'wb')

    def write_file_magic(self, version = 1):
        self.file.write(b'VWFB')
        self.file.write(struct.pack('I', version))

    def write_file_header(self, properties):
        builder = flatbuffers.Builder(0)
        kv_offsets = []
        for key in properties:
            value = properties[key]
            k_off = builder.CreateString(str(key))
            v_off = builder.CreateString(str(value))
            KeyValueStart(builder)
            KeyValueAddKey(builder, k_off)
            KeyValueAddValue(builder, v_off)
            kv_offsets.append(KeyValueEnd(builder))

        props_off = mk_offsets_vector(builder, kv_offsets, FileHeaderStartPropertiesVector)

        FileHeaderStart(builder)
        FileHeaderAddJoinTime(builder, mk_timestamp(builder))
        FileHeaderAddProperties(builder, props_off)

        header_off = FileHeaderEnd(builder)
        builder.Finish(header_off)
        self._write_message(MSG_TYPE_HEADER, builder.Output())

    def write_regular_message(self, events):
        builder = flatbuffers.Builder(0)

        evt_offsets = []
        for evt in events:
            payload_off = mk_bytes_vector(builder, evt.serialize())
            JoinedEventStart(builder)
            JoinedEventAddEvent(builder, payload_off)
            JoinedEventAddTimestamp(builder, mk_timestamp(builder))
            evt_offsets.append(JoinedEventEnd(builder))

        evt_array_offset = mk_offsets_vector(builder, evt_offsets, JoinedPayloadStartEventsVector)

        JoinedPayloadStart(builder)
        JoinedPayloadAddEvents(builder, evt_array_offset)
        joined_payload_off = JoinedPayloadEnd(builder)

        builder.Finish(joined_payload_off)
        self._write_message(MSG_TYPE_REGULAR, builder.Output())

    def write_checkpoint_info(self, reward_fun = RewardFunctionType.Earliest, default_reward = 0.0, learning_mode = LearningModeType.Online, problem_type = ProblemType.CB, use_client_time = False):
        builder = flatbuffers.Builder(0)

        CheckpointInfoStart(builder)
        CheckpointInfoAddRewardFunctionType(builder, reward_fun)
        CheckpointInfoAddDefaultReward(builder, default_reward)
        CheckpointInfoAddLearningModeConfig(builder, learning_mode)
        CheckpointInfoAddProblemTypeConfig(builder, problem_type)
        CheckpointInfoAddUseClientTime(builder, use_client_time)

        checkpoint_info_off = CheckpointInfoEnd(builder)
        builder.Finish(checkpoint_info_off)
        self._write_message(MSG_TYPE_CHECKPOINT, builder.Output())

    def write_eof(self):
        self._write_message(MSG_TYPE_EOF, b'')

    def _write_message(self, kind, payload):
        padding_bytes = len(payload) % 8

        self.file.write(struct.pack('I', kind))
        self.file.write(struct.pack('I', len(payload)))
        self.file.write(payload)
        if padding_bytes > 0:
            self.file.write(bytes([0] * padding_bytes))

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        return self.file.__exit__(type, value, traceback)

def main():
    if len(sys.argv) != 2:
        print("""Usage: log_gen.py <XXXX>
    Where XXXX is a sequence of letters telling which kind of message to output next:
    m -> File Magic
    h -> File Header
    c -> Checkpoint message
    r -> Single CB message with no observation
    For example: ./log_gen.py mhcrr
    This writes to output.fb five messages in sequence: file magic, file header, checkpoint, CB and CB.
        """)
    else:
        foo = BinLogWriter('output.fb')

        for c in sys.argv[1]:
            if   c == 'm':
                foo.write_file_magic()
            elif c == 'h':
                foo.write_file_header({ 'foo': 'bar'})
            elif c == 'c':
                foo.write_checkpoint_info()
            elif c == 'r':
                foo.write_regular_message([mk_cb_payload()])
            else:
                print(f'Invalid character \'{c}\'. Ignoring it.')

if __name__ == "__main__":
    main()