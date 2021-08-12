#! /usr/bin/env python3 -W ignore::DeprecationWarning

from reinforcement_learning.messages.flatbuff.v2.IndexValue import IndexValue
from reinforcement_learning.messages.flatbuff.v2.EventBatch import *
from reinforcement_learning.messages.flatbuff.v2.BatchMetadata import *
from reinforcement_learning.messages.flatbuff.v2.EventBatch import *
from reinforcement_learning.messages.flatbuff.v2.LearningModeType import LearningModeType
from reinforcement_learning.messages.flatbuff.v2.PayloadType import PayloadType
from reinforcement_learning.messages.flatbuff.v2.OutcomeValue import OutcomeValue
from reinforcement_learning.messages.flatbuff.v2.NumericOutcome import NumericOutcome, NumericOutcomeAddValue, NumericOutcomeEnd, NumericOutcomeStart
from reinforcement_learning.messages.flatbuff.v2.NumericIndex import *

from reinforcement_learning.messages.flatbuff.v2.Metadata import *
from reinforcement_learning.messages.flatbuff.v2.CbEvent import *
from reinforcement_learning.messages.flatbuff.v2.OutcomeEvent import *
from reinforcement_learning.messages.flatbuff.v2.MultiSlotEvent import MultiSlotEvent
from reinforcement_learning.messages.flatbuff.v2.MultiStepEvent import *
from reinforcement_learning.messages.flatbuff.v2.CaEvent import CaEvent
from reinforcement_learning.messages.flatbuff.v2.DedupInfo import DedupInfo

from reinforcement_learning.messages.flatbuff.v2.KeyValue import *
from reinforcement_learning.messages.flatbuff.v2.TimeStamp import *
from reinforcement_learning.messages.flatbuff.v2.FileHeader import *
from reinforcement_learning.messages.flatbuff.v2.JoinedEvent import *
from reinforcement_learning.messages.flatbuff.v2.JoinedPayload import *
from reinforcement_learning.messages.flatbuff.v2.Metadata import *
from reinforcement_learning.messages.flatbuff.v2.Event import *
from reinforcement_learning.messages.flatbuff.v2.CheckpointInfo import *
from reinforcement_learning.messages.flatbuff.v2.RewardFunctionType import *
from reinforcement_learning.messages.flatbuff.v2.ProblemType import *
from reinforcement_learning.messages.flatbuff.v2.EventEncoding import *

import flatbuffers
import sys
import json
import struct
from datetime import datetime
import numpy as np
import argparse


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
    time = datetime.utcnow()
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
            payload_off = mk_bytes_vector(builder, evt)
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


def mk_long_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype=np.int64))

def mk_float_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype=np.float32))



CB_STR = """{"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}]}"""

def mk_cb_payload(_event_id='event_id_0', _actions=[1,3], _ctx=CB_STR, _probs=[0.1,0.9],
        _deferred=False, _model_id='the model', _learning_mode=LearningModeType.Online, _pdrop = 0):
    # first gen the CB payload
    builder = flatbuffers.Builder(0)

    actions = mk_long_vector(builder, _actions)
    ctx = mk_bytes_vector(builder, bytes(_ctx, 'utf-8'))
    probs = mk_float_vector(builder, _probs)
    model_id = builder.CreateString(_model_id)

    CbEventStart(builder)
    CbEventAddDeferredAction(builder, _deferred)
    CbEventAddActionIds(builder, actions)
    CbEventAddContext(builder, ctx)
    CbEventAddProbabilities(builder, probs)
    CbEventAddModelId(builder, model_id)
    CbEventAddLearningMode(builder, _learning_mode)
    builder.Finish(CbEventEnd(builder))
    
    cb_payload = builder.Output()

    builder = flatbuffers.Builder(0)
    event_id = builder.CreateString(_event_id)

    MetadataStart(builder)
    MetadataAddId(builder, event_id)
    MetadataAddClientTimeUtc(builder, mk_timestamp(builder))
    MetadataAddPayloadType(builder, PayloadType.CB)
    MetadataAddEncoding(builder, EventEncoding.Identity)
    MetadataAddPassProbability(builder=builder, passProbability = 1 - _pdrop)

    md_off = MetadataEnd(builder)
    payload_off = mk_bytes_vector(builder, cb_payload)

    EventStart(builder)
    EventAddMeta(builder, md_off)
    EventAddPayload(builder, payload_off)
    builder.Finish(EventEnd(builder))

    return builder.Output()

def mk_multistep_payload(_episode_id='episode_0', _event_id='0', _previous_id=None, _actions=[1,2], _ctx=CB_STR, _probs=[0.1,0.9],
        _deferred=False, _model_id='the model', _pdrop = 0):
    # first gen the CB payload
    builder = flatbuffers.Builder(0)

    actions = mk_long_vector(builder, _actions)
    ctx = mk_bytes_vector(builder, bytes(_ctx, 'utf-8'))
    probs = mk_float_vector(builder, _probs)
    model_id = builder.CreateString(_model_id)
    event_id = builder.CreateString(_event_id)
    previous_id = builder.CreateString(_previous_id) if _previous_id is not None else None

    MultiStepEventStart(builder)
    MultiStepEventAddEventId(builder, event_id)
    if previous_id is not None:
        MultiStepEventAddPreviousId(builder, previous_id)
    MultiStepEventAddDeferredAction(builder, _deferred)
    MultiStepEventAddActionIds(builder, actions)
    MultiStepEventAddContext(builder, ctx)
    MultiStepEventAddProbabilities(builder, probs)
    MultiStepEventAddModelId(builder, model_id)
    builder.Finish(MultiStepEventEnd(builder))
    
    cb_payload = builder.Output()

    builder = flatbuffers.Builder(0)
    episode_id = builder.CreateString(_episode_id)

    MetadataStart(builder)
    MetadataAddId(builder, episode_id)
    MetadataAddClientTimeUtc(builder, mk_timestamp(builder))
    MetadataAddPayloadType(builder, PayloadType.MultiStep)
    MetadataAddEncoding(builder, EventEncoding.Identity)
    MetadataAddPassProbability(builder=builder, passProbability = 1 - _pdrop)

    md_off = MetadataEnd(builder)
    payload_off = mk_bytes_vector(builder, cb_payload)

    EventStart(builder)
    EventAddMeta(builder, md_off)
    EventAddPayload(builder, payload_off)
    builder.Finish(EventEnd(builder))

    return builder.Output()

def mk_outcome(_primary_id='event_id_0', _secondary_id=None, _value=1, _pdrop=0):
    if isinstance(_value, (int, float)):
        value_type = OutcomeValue.numeric
    elif isinstance(_value, str):
        value_type = OutcomeValue.literal
    elif _value is None:
        value_type = OutcomeValue.NONE
    else:
        raise 'Unknown value type'

    if _secondary_id is None:
        index_type = IndexValue.NONE
    elif isinstance(_secondary_id, (int, float)):
        index_type = IndexValue.numeric
    elif isinstance(_secondary_id, str):
        index_type = IndexValue.literal
    else:
        raise 'Unknown index type'

    builder = flatbuffers.Builder(0)
    
    outcome = None
    if value_type == OutcomeValue.numeric:
        NumericOutcomeStart(builder)
        NumericOutcomeAddValue(builder, _value)
        outcome = NumericOutcomeEnd(builder)
    elif value_type == OutcomeValue.literal:
        outcome = builder.CreateString(_value)

    index = None
    if index_type == IndexValue.numeric:
        NumericIndexStart(builder)
        NumericIndexAddIndex(builder, _secondary_id)
        index = NumericIndexEnd(builder)
    elif index_type == OutcomeValue.literal:
        index = builder.CreateString(_secondary_id)

    OutcomeEventStart(builder)
    OutcomeEventAddIndexType(builder, index_type)
    if index_type != IndexValue.NONE:
        OutcomeEventAddIndex(builder, index)

    OutcomeEventAddValueType(builder, value_type)
    if value_type != OutcomeValue.NONE:
        OutcomeEventAddValue(builder, outcome)
    OutcomeEventAddActionTaken(builder, value_type == OutcomeValue.NONE)

    builder.Finish(OutcomeEventEnd(builder))
    
    reward_payload = builder.Output()

    builder = flatbuffers.Builder(0)
    event_id = builder.CreateString(_primary_id)

    MetadataStart(builder)
    MetadataAddId(builder, event_id)
    MetadataAddClientTimeUtc(builder, mk_timestamp(builder))
    MetadataAddPayloadType(builder, PayloadType.Outcome)
    MetadataAddEncoding(builder, EventEncoding.Identity)
    MetadataAddPassProbability(builder=builder, passProbability = 1 - _pdrop)

    md_off = MetadataEnd(builder)
    payload_off = mk_bytes_vector(builder, reward_payload)

    EventStart(builder)
    EventAddMeta(builder, md_off)
    EventAddPayload(builder, payload_off)
    builder.Finish(EventEnd(builder))

    return builder.Output()

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