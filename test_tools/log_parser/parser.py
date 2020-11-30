#! /usr/bin/env python3 -W ignore::DeprecationWarning
import flatbuffers
import zstd
import sys
import json

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

def payload_name(payload):
    for k in [f for f in dir(PayloadType) if not f.startswith('__')]:
        if getattr(PayloadType, k) == payload:
            return k
    return f'<unk_{payload}>'

def learning_mode_name(learning_mode):
    for k in [f for f in dir(LearningModeType) if not f.startswith('__')]:
        if getattr(LearningModeType, k) == learning_mode:
            return k
    return f'<unk_{learning_mode}>'

def process_payload(payload, is_dedup):
    if not is_dedup:
        return payload
    return zstd.decompress(payload)

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

    print(f'\tmulti-slot slots:{evt.SlotsLength()} model:{evt.ModelId()} deferred:{evt.DeferredAction()}')
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

def dump_event(evt, idx, is_dedup):
    m = evt.Meta()
    print(f'\t[{idx}] id:{m.Id().decode("utf-8")} type:{payload_name(m.PayloadType())} payload-size:{evt.PayloadLength()}')

    payload = process_payload(evt.PayloadAsNumpy(), is_dedup)

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
    else:
        print('unknown payload type')

def dump_file(f):
    buf = open(f, 'rb').read()
    buf = bytearray(buf)
    preamble = parse_preamble(buf)

    batch = EventBatch.GetRootAsEventBatch(buf[PREAMBLE_LENGTH : PREAMBLE_LENGTH + preamble["msg_size"]], 0)
    meta = batch.Metadata()
    print(f'parsed {f} with {batch.EventsLength()} events preamble:{preamble} enc:{meta.ContentEncoding()}')
    is_dedup = b'ZSTD_AND_DEDUP' == meta.ContentEncoding()
    for i in range(0, batch.EventsLength()):
        dump_event(batch.Events(i), i, is_dedup)
    print("----\n")

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

for f in sys.argv[1:]:
    dump_file(f)
