#! /usr/bin/env python3 -W ignore::DeprecationWarning
from os import times
import flatbuffers
import zstd
import sys
import json
import struct
import datetime

PREAMBLE_LENGTH = 8
PRETTY_PRINT_JSON=False


def fmt_payload(payload):
    payload = bytearray(payload).decode('utf-8')
    if PRETTY_PRINT_JSON == False:
        return payload
    return json.dumps(json.loads(payload), indent = 1)

def parse_preamble(buf):
    reserved = buf[0]
    version = buf[1]
    msg_type = int.from_bytes(buf[2:4], "big")
    msg_size = int.from_bytes(buf[4:8], "big")
    return { 'reserved': reserved, 'version': version, 'msg_type': msg_type, 'msg_size': msg_size}


def enum_to_str(type, value):
    for k in [f for f in dir(type) if not f.startswith('__')]:
        if getattr(type, k) == value:
            return k
    return f'<unk_{value}>'

def payload_name(payload):
    return enum_to_str(PayloadType, payload)

def learning_mode_name(learning_mode):
    return enum_to_str(LearningModeType, learning_mode)

def event_encoding_name(batch_type):
    return enum_to_str(EventEncoding, batch_type)

def timestamp_to_datetime(timestamp):
    if timestamp == None:
        return None
    return datetime.datetime(timestamp.Year(), timestamp.Month(), timestamp.Day(), timestamp.Hour(), timestamp.Minute(), timestamp.Second(), timestamp.Subsecond())

# Similar hack to the C# one due to limited binding codegen
def getString(table):
    off = table.Pos
    length = flatbuffers.encode.Get(flatbuffers.number_types.UOffsetTFlags.packer_type, table.Bytes, off)
    start = off + flatbuffers.number_types.UOffsetTFlags.bytewidth
    return bytes(table.Bytes[start:start+length])

def cast(table, tmp_type):
    tmp = tmp_type()
    tmp.Init(table.Bytes, table.Pos)
    return tmp


def parse_cb(payload):
    evt = CbEvent.GetRootAsCbEvent(payload, 0)
    print(f'\tcb: actions:{evt.ActionIdsLength()} model:{evt.ModelId()} lm:{learning_mode_name(evt.LearningMode())} deferred:{evt.DeferredAction()}')
    print(f'\t\tcontext: {fmt_payload(evt.ContextAsNumpy())}')

def parse_outcome(payload):
    evt = OutcomeEvent.GetRootAsOutcomeEvent(payload, 0)

    value = evt.Value()
    if evt.ValueType() == OutcomeValue.literal:
        value = getString(value)
    elif evt.ValueType() == OutcomeValue.numeric:
        value = cast(value, NumericOutcome).Value()

    index = evt.Index()
    if evt.IndexType() == OutcomeValue.literal:
        index = getString(index)
    elif evt.IndexType() == OutcomeValue.numeric:
        index = cast(index, NumericIndex).Index()

    print(f'\toutcome: value:{value} index:{index} action-taken:{evt.ActionTaken()}')

def parse_multislot(payload):
    evt = MultiSlotEvent.GetRootAsMultiSlotEvent(payload, 0)

    print(f'\tmulti-slot slots:{evt.SlotsLength()} model:{evt.ModelId()} deferred:{evt.DeferredAction()} has-baseline:{not evt.BaselineActionsIsNone()}')
    print(f'\t\tcontext: {fmt_payload(evt.ContextAsNumpy())}')
    if not evt.BaselineActionsIsNone():
        print(f'\t\tbaselines: {" ".join([str(b) for b in evt.BaselineActionsAsNumpy()])}')

def parse_multistep(payload):
    evt = MultiStepEvent.GetRootAsMultiStepEvent(payload, 0)

    print(f'\tmultistep: index: {evt.EventId()}\t actions:{evt.ActionIdsLength()} model:{evt.ModelId()}')
    print(f'\t\tcontext: {fmt_payload(evt.ContextAsNumpy())}')


def parse_continuous_action(payload):
    evt = CaEvent.GetRootAsCaEvent(payload, 0)

    print(f'\tcontinuous-action: action:{evt.Action()} pdf-value:{evt.PdfValue()} deferred:{evt.DeferredAction()}')
    print(f'\t\tcontext: {fmt_payload(evt.ContextAsNumpy())}')

def parse_dedup_info(payload):
    evt = DedupInfo.GetRootAsDedupInfo(payload, 0)
    print(f'\tdedup-info ids:{evt.IdsLength()} values:{evt.ValuesLength()}')
    for i in range(0, evt.ValuesLength()):
        print(f'\t\t[{evt.Ids(i)}]: "{evt.Values(i).decode("utf-8")}"')

def dump_event(event_payload, idx, timestamp=None):
    evt = Event.GetRootAsEvent(event_payload, 0)
    m = evt.Meta()

    print(f'\t[{idx}] id:{m.Id().decode("utf-8")} type:{payload_name(m.PayloadType())} payload-size:{evt.PayloadLength()} encoding:{event_encoding_name(m.Encoding())} ts:{timestamp_to_datetime(timestamp)}')

    payload = evt.PayloadAsNumpy()
    if m.Encoding() == EventEncoding.Zstd:
        payload = zstd.decompress(evt.PayloadAsNumpy())

    if m.PayloadType() == PayloadType.CB:
        parse_cb(payload)
    elif m.PayloadType() == PayloadType.CCB or m.PayloadType() == PayloadType.Slates:
        parse_multislot(payload)
    elif m.PayloadType() == PayloadType.Outcome:
        parse_outcome(payload)
    elif m.PayloadType() == PayloadType.CA:
        parse_continuous_action(payload)
    elif m.PayloadType() == PayloadType.DedupInfo:
        parse_dedup_info(payload)
    elif m.PayloadType() == PayloadType.MultiStep:
        parse_multistep(payload)
    else:
        print('unknown payload type')


def dump_event_batch(buf):
    batch = EventBatch.GetRootAsEventBatch(buf, 0)
    meta = batch.Metadata()
    enc = meta.ContentEncoding().decode('utf-8')
    print(f'event-batch evt-count:{batch.EventsLength()} enc:{enc}')
    is_dedup = b'DEDUP' == meta.ContentEncoding()
    for i in range(0, batch.EventsLength()):
        dump_event(batch.Events(i).PayloadAsNumpy(), i)
    print("----\n")


def dump_preamble_file(file_name, buf):
    preamble = parse_preamble(buf)
    print(f'parsing preamble file {file_name}\n\tpreamble:{preamble}')
    dump_event_batch(buf[PREAMBLE_LENGTH : PREAMBLE_LENGTH + preamble["msg_size"]])

MSG_TYPE_HEADER = 0x55555555
MSG_TYPE_CHECKPOINT = 0x11111111
MSG_TYPE_REGULAR = 0xFFFFFFFF
MSG_TYPE_EOF = 0xAAAAAAAA

class JoinedLogStreamReader:
    def __init__(self, buf):
        self.buf = buf
        self.offset = 0
        self.headers = dict()
        self.parse_header()
        self.read_file_header()

    def parse_header(self):
        if self.buf[0:4] != b'VWFB':
            raise Exception("Invalid file magic")

        self.version = struct.unpack('I', self.buf[4:8])[0]
        if self.version != 1:
            raise Exception(f'Unsuported file version {self.version}')
        self.offset = 8

    def read(self, size):
        if size == 0:
            return bytearray([])
        data = self.buf[self.offset : self.offset + size]
        self.offset += size
        return data

    def read_message(self):
        kind = struct.unpack('I', self.read(4))[0]
        length = struct.unpack('I', self.read(4))[0]
        payload = self.read(length)
        #discard padding
        self.read(length % 8)
        return (kind, payload)

    def read_file_header(self):
        msg = self.read_message()
        if msg[0] != MSG_TYPE_HEADER:
            raise f'Missing file header, found message of type {msg[0]} instead'

        header = FileHeader.GetRootAsFileHeader(msg[1], 0)
        for i in range(header.PropertiesLength()):
            p = header.Properties(i)
            self.headers[p.Key().decode('utf-8')] = p.Value().decode('utf-8')

    def checkpoint_info(self):
        msg = self.read_message()
        if msg[0] != MSG_TYPE_CHECKPOINT:
            raise f'Missing checkpoint info, found message type of {msg[0]} instead'
        return CheckpointInfo.GetRootAsCheckpointInfo(msg[1], 0)

    def messages(self):
        while True:
            msg = self.read_message()
            if msg[0] == MSG_TYPE_EOF:
                break
            yield JoinedPayload.GetRootAsJoinedPayload(msg[1], 0)

def dump_joined_log_file(file_name, buf):
    reader = JoinedLogStreamReader(buf)
    print(f'parsing joined log:{file_name} header:')
    for k in reader.headers:
        print(f'\t{k} = {reader.headers[k]}')

    checkpoint_info = reader.checkpoint_info()
    print(f'reward function type is: {checkpoint_info.RewardFunctionType()}')
    print(f'default reward is: {checkpoint_info.DefaultReward()}')
    print(f'learning mode config is: {checkpoint_info.LearningModeConfig()}')
    print(f'problem type config is: {checkpoint_info.ProblemTypeConfig()}')

    for msg in reader.messages():
        print(f'joined-batch events: {msg.EventsLength()}')
        for i in range(msg.EventsLength()):
            joined_event = msg.Events(i)
            dump_event(joined_event.EventAsNumpy(), i, joined_event.Timestamp())

def dump_file(f):
    buf = bytearray(open(f, 'rb').read())

    if buf[0:4] == b'VWFB':
        dump_joined_log_file(f, buf)
    else:
        dump_preamble_file(f, buf)


# Generate FB serializers if they are not available
try:
    import reinforcement_learning.messages.flatbuff.v2.EventBatch
except Exception as e:
    import pathlib
    import subprocess

    script_dir = pathlib.Path(__file__).parent.absolute()
    input_dir = pathlib.Path(script_dir).parents[1].joinpath('rlclientlib', 'schema', 'v2')

    input_files = " ".join([str(x) for x in input_dir.glob('*.fbs')])
    subprocess.run(f'flatc --python {input_files}', cwd=script_dir, shell=True, check=True)


# must be done after the above that generates the classes we're importing
from reinforcement_learning.messages.flatbuff.v2.EventBatch import EventBatch
from reinforcement_learning.messages.flatbuff.v2.Event import Event
from reinforcement_learning.messages.flatbuff.v2.EventEncoding import EventEncoding
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
from reinforcement_learning.messages.flatbuff.v2.MultiStepEvent import MultiStepEvent

from reinforcement_learning.messages.flatbuff.v2.FileHeader import *
from reinforcement_learning.messages.flatbuff.v2.JoinedEvent import *
from reinforcement_learning.messages.flatbuff.v2.JoinedPayload import *
from reinforcement_learning.messages.flatbuff.v2.CheckpointInfo import *
from reinforcement_learning.messages.flatbuff.v2.ProblemType import *

for input_file in sys.argv[1:]:
    dump_file(input_file)
