#! /usr/bin/env python3 -W ignore::DeprecationWarning
import reinforcement_learning.messages.flatbuff.v2.CbEvent as cb
import reinforcement_learning.messages.flatbuff.v2.ActionDictionary as ad
import reinforcement_learning.messages.flatbuff.v2.EventBatch as eb
import reinforcement_learning.messages.flatbuff.v2.PayloadType as pt
import reinforcement_learning.messages.flatbuff.v2.LearningModeType as lm
import flatbuffers
import zstandard as zstd
import sys
import json

PREAMBLE_LENGTH = 8

def parse_preamble(buf):
    reserved = buf[0]
    version = buf[1]
    msg_type = int.from_bytes(buf[2:4], "big")
    msg_size = int.from_bytes(buf[4:8], "big")
    return { 'reserved': reserved, 'version': version, 'msg_type': msg_type, 'msg_size': msg_size}

def payload_name(payload):
    for k in [f for f in dir(pt.PayloadType) if not f.startswith('__')]:
        if getattr(pt.PayloadType, k) == payload:
            return k
    return f'<unk_{payload}>'

def learning_mode_name(learning_mode):
    for k in [f for f in dir(lm.LearningModeType) if not f.startswith('__')]:
        if getattr(lm.LearningModeType, k) == learning_mode:
            return k
    return f'<unk_{learning_mode}>'


def process_payload(payload, is_dedup):
    if not is_dedup:
        return payload
    return zstd.decompress(payload)

def parse_cb(payload):
    evt = cb.CbEvent.GetRootAsCbEvent(payload, 0)
    print(f'\tcb: actions:{evt.ActionIdsLength()} ctx:{evt.ContextLength()} probs:{evt.ProbabilitiesLength()} lm:{learning_mode_name(evt.LearningMode())}')
    payload = json.loads(bytearray(evt.ContextAsNumpy()).decode('utf-8'))
    print('\tcontext:')
    print(json.dumps(payload, indent = 1))

def parse_action_dict(payload):
    evt = ad.ActionDictionary.GetRootAsActionDictionary(payload, 0)
    print(f'\t\tad: ids:{evt.IdsLength()} values:{evt.ValuesLength()}')
    for i in range(0, evt.ValuesLength()):
        print(f'\t\t\t[{evt.Ids(i)}]: "{evt.Values(i)}"')

def dump_event(evt, idx, is_dedup):
    m = evt.Meta()
    print(f'\t[{idx}] id:{m.Id()} type:{payload_name(m.PayloadType())} payload-size:{evt.PayloadLength()}')

    payload = process_payload(evt.PayloadAsNumpy(), is_dedup)

    if m.PayloadType() == pt.PayloadType.DedupInfo:
        parse_action_dict(payload)
    elif m.PayloadType() == pt.PayloadType.CB:
        parse_cb(payload)


def dump_file(f):
    buf = open(f, 'rb').read()
    buf = bytearray(buf)
    preamble = parse_preamble(buf)

    batch = eb.EventBatch.GetRootAsEventBatch(buf[PREAMBLE_LENGTH : PREAMBLE_LENGTH + preamble["msg_size"]], 0)
    meta = batch.Metadata()
    print(f'parsed {f} with {batch.EventsLength()} events preamble:{preamble} enc:{meta.ContentEncoding()}')
    is_dedup = b'ZSTD_AND_DEDUP' == meta.ContentEncoding()
    for i in range(0, batch.EventsLength()):
        dump_event(batch.Events(i), i, is_dedup)
    print("----\n")

for f in sys.argv[1:]:
    dump_file(f)
