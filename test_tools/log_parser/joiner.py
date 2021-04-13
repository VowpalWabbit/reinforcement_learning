#! /usr/bin/env python3 -W ignore::DeprecationWarning

from reinforcement_learning.messages.flatbuff.v2.EventBatch import *
from reinforcement_learning.messages.flatbuff.v2.BatchMetadata import *
from reinforcement_learning.messages.flatbuff.v2.EventBatch import *
from reinforcement_learning.messages.flatbuff.v2.LearningModeType import LearningModeType
from reinforcement_learning.messages.flatbuff.v2.PayloadType import PayloadType
from reinforcement_learning.messages.flatbuff.v2.OutcomeValue import OutcomeValue
from reinforcement_learning.messages.flatbuff.v2.NumericOutcome import NumericOutcome
from reinforcement_learning.messages.flatbuff.v2.NumericIndex import NumericIndex

from reinforcement_learning.messages.flatbuff.v2.CbEvent import CbEvent
from reinforcement_learning.messages.flatbuff.v2.OutcomeEvent import OutcomeEvent
from reinforcement_learning.messages.flatbuff.v2.MultiSlotEvent import MultiSlotEvent
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

import flatbuffers
import zstandard as zstd
import sys
import json
import struct
from datetime import datetime
import numpy as np
import argparse

"""
TODO:
Incremental join instead of loading all interactions at once
Respect EUD.
"""

arg_parser = argparse.ArgumentParser()

arg_parser.add_argument(
    '--reward_function',
    type=int,
    choices=[value for name, value in RewardFunctionType.__dict__.items()],
    help = "reward function type: 0-earliest, 1-avg, 2-median, 3-sum, 4-min, 5-max"
)
arg_parser.add_argument('--default_reward', type=float, help="default reward")
arg_parser.add_argument(
    '--learning_mode_config',
    type=int,
    choices=[value for name, value in LearningModeType.__dict__.items()],
    help = "learning mode type: 0-Online, 1-Apprentice, 2-LoggingOnly"
)
arg_parser.add_argument(
    '--problem_type_config',
    type=int,
    choices=[value for name, value in ProblemType.__dict__.items()],
    help = 'problem type:  0-Unknown, 1-CB, 2-CCB, 3-SLATES, 4-CA'
)
arg_parser.add_argument('-c', action='store_true', help='generate corrupt files for binary parser unit tests')

args = arg_parser.parse_args()

reward_function_type = RewardFunctionType.Earliest
default_reward = 0
learning_mode_config = LearningModeType.Online
problem_type_config = ProblemType.UNKNOWN

if args.reward_function:
    reward_function_type = args.reward_function

if args.default_reward:
    default_reward = args.default_reward

if args.learning_mode_config:
    learning_mode_config = args.learning_mode_config

if args.problem_type_config:
    problem_type_config = args.problem_type_config

class PreambleStreamReader:
    def __init__(self, file_name):
        self.file = open(file_name, 'rb')

    def parse_preamble(self):
        buf = self.file.read(8)
        if buf == b'':
            return None

        reserved = buf[0]
        version = buf[1]
        msg_type = int.from_bytes(buf[2:4], "big")
        msg_size = int.from_bytes(buf[4:8], "big")
        return { 'reserved': reserved, 'version': version, 'msg_type': msg_type, 'msg_size': msg_size}

    def messages(self):
        while True:
            header = self.parse_preamble()
            if header == None:
                break
            msg = self.file.read(header['msg_size'])
            # yield (EventBatch.GetRootAsEventBatch(msg, 0), msg)
            yield msg


def mk_timestamp(builder):
    time = datetime.utcnow()
    return CreateTimeStamp(builder, time.year, time.month, time.day, time.hour, time.minute, time.second, int(time.microsecond))

def mk_offsets_vector(builder, arr, startFun):
    startFun(builder, len(arr))
    for i in reversed(range(len(arr))):
        builder.PrependUOffsetTRelative(arr[i])
    return builder.EndVector(len(arr))

def mk_bytes_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype='b'))

MSG_TYPE_HEADER = 0x55555555
MSG_TYPE_CHECKPOINT = 0x11111111
MSG_TYPE_REGULAR = 0xFFFFFFFF
MSG_TYPE_EOF = 0xAAAAAAAA

class BinLogWriter:
    def __init__(self, file_name):
        self.file = open(file_name, 'wb')

    def write_message(self, kind, payload):
        padding_bytes = len(payload) % 8
        print(f'msg {kind:X} size: {len(payload)} padding {padding_bytes}')

        self.file.write(struct.pack('I', kind))
        self.file.write(struct.pack('I', len(payload)))
        self.file.write(payload)
        if padding_bytes > 0:
            self.file.write(bytes([0] * padding_bytes))

    def write_header(self, properties, write_header=True):
        self.file.write(b'VWFB')
        self.file.write(struct.pack('I', 1))

        if write_header:
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
            self.write_message(MSG_TYPE_HEADER, builder.Output())

    def write_join_msg(self, events, mess_with_payload = False):
        builder = flatbuffers.Builder(0)

        evt_offsets = []
        for evt in events:
            payload_off = mk_bytes_vector(builder, evt)
            JoinedEventStart(builder)
            JoinedEventAddEvent(builder, payload_off)
            JoinedEventAddTimestamp(builder, mk_timestamp(builder))
            evt_offsets.append(JoinedEventEnd(builder))
            if mess_with_payload:
                mess_with_payload = False
                evt_offsets.append(0)


        evt_array_offset = mk_offsets_vector(builder, evt_offsets, JoinedPayloadStartEventsVector)

        JoinedPayloadStart(builder)
        JoinedPayloadAddEvents(builder, evt_array_offset)
        joined_payload_off = JoinedPayloadEnd(builder)

        builder.Finish(joined_payload_off)
        self.write_message(MSG_TYPE_REGULAR, builder.Output())

    def write_checkpoint_info(self):
        print("reward function type: ", reward_function_type)
        print("default reward: ", default_reward)
        print("learning_mode_config: ", learning_mode_config)
        print("problem_type_config: ", problem_type_config)
        builder = flatbuffers.Builder(0)

        CheckpointInfoStart(builder)
        CheckpointInfoAddRewardFunctionType(builder, reward_function_type)
        CheckpointInfoAddDefaultReward(builder, default_reward)
        CheckpointInfoAddLearningModeConfig(builder, learning_mode_config)
        CheckpointInfoAddProblemTypeConfig(builder, problem_type_config)

        checkpoint_info_off = CheckpointInfoEnd(builder)
        builder.Finish(checkpoint_info_off)
        self.write_message(MSG_TYPE_CHECKPOINT, builder.Output())

    def write_eof(self):
        self.write_message(MSG_TYPE_EOF, b'')

interaction_file_name = 'cb_v2.fb'
observations_file_name = 'f-reward_v2.fb'
interactions_file = PreambleStreamReader(interaction_file_name)
observations_file = PreambleStreamReader(observations_file_name)

result_file = 'merged.log'

def get_event_id(ser_evt):
    evt = Event.GetRootAsEvent(ser_evt.PayloadAsNumpy(), 0)
    m = evt.Meta()
    if m:
        return m.Id()
    return None


observations = dict()
obs_count = 0
obs_ids = 0
for msg in observations_file.messages():
    batch = EventBatch.GetRootAsEventBatch(msg, 0)
    for i in range(0, batch.EventsLength()):
        ser_evt = batch.Events(i)
        evt_id = get_event_id(ser_evt)
        if evt_id not in observations:
            observations[evt_id] = []
            obs_ids += 1
        observations[evt_id].append(ser_evt.PayloadAsNumpy())
        obs_count +=1

print(f'found {obs_count} observations with {obs_ids} ids')

bin_f = BinLogWriter(result_file)
bin_f.write_header({ 'eud': '-1', 'joiner': 'joiner.py'})
bin_f.write_checkpoint_info()

for msg in interactions_file.messages():
    batch = EventBatch.GetRootAsEventBatch(msg, 0)
    #collect all observations
    events_to_serialize = []
    for i in range(0, batch.EventsLength()):
        ser_evt = batch.Events(i)
        events_to_serialize.append(ser_evt.PayloadAsNumpy())
        evt_id = get_event_id(ser_evt)
        if evt_id in observations:
            for obs in observations[evt_id]:
                events_to_serialize.append(obs)

    print(f'batch with {len(events_to_serialize)} events')
    print(f'joining iters with {batch.Metadata().ContentEncoding()}')
    bin_f.write_join_msg(events_to_serialize)

bin_f.write_eof()

if args.c:
    print()
    print("---> Generating corrupt files for testing <---")
    print()

    # re-read interactions file
    interactions_file = PreambleStreamReader(interaction_file_name)

    result_file_bad_magic = 'bad_magic.log'
    file_bm = open(result_file_bad_magic, 'wb')
    file_bm.write(b'VWFC')

    result_file_bad_version = 'bad_version.log'
    file_bv = open(result_file_bad_version, 'wb')
    file_bv.write(b'VWFB')
    file_bv.write(struct.pack('I', 2))

    result_file_empty_msg_hdr = 'empty_msg_hdr.log'
    bin_f_eh = BinLogWriter(result_file_empty_msg_hdr)
    bin_f_eh.write_header({})
    bin_f_eh.write_checkpoint_info()

    # assuming this should not be missing
    result_file_no_msg_hdr = 'no_msg_hdr.log'
    bin_f_nh = BinLogWriter(result_file_no_msg_hdr)
    bin_f_nh.write_header({}, write_header=False)

    result_file_unknown_msg_type = 'unknown_msg_type.log'
    bin_f_umt = BinLogWriter(result_file_unknown_msg_type)
    bin_f_umt.write_header({ 'eud': '-1', 'joiner': 'joiner.py'})
    bin_f_umt.write_checkpoint_info()
    MSG_TYPE_UNKNOWN = 0x1AAAAAAA
    bin_f_umt.write_message(MSG_TYPE_UNKNOWN, b'')

    result_file_bad_payload = 'bad_joined_payload.log'

    bin_bad_payload = BinLogWriter(result_file_bad_payload)
    bin_bad_payload.write_header({ 'eud': '-1', 'joiner': 'joiner.py'})
    bin_bad_payload.write_checkpoint_info()

    mess_with_payload = True
    for msg in interactions_file.messages():
        batch = EventBatch.GetRootAsEventBatch(msg, 0)
        #collect all observations
        for i in range(0, batch.EventsLength()):
            events_to_serialize = []
            ser_evt = batch.Events(i)
            events_to_serialize.append(ser_evt.PayloadAsNumpy())
            evt_id = get_event_id(ser_evt)
            if evt_id in observations:
                for obs in observations[evt_id]:
                    events_to_serialize.append(obs)

            # make two batches here
            print(f'batch with {len(events_to_serialize)} events')
            print(f'joining iters with {batch.Metadata().ContentEncoding()}')
            bin_bad_payload.write_join_msg(events_to_serialize, mess_with_payload=mess_with_payload)
            mess_with_payload = False # only mess with the first payload

    bin_bad_payload.write_eof()