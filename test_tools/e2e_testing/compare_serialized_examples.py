#! /usr/bin/env python3 -W ignore::DeprecationWarning
import argparse
import flatbuffers
import numpy as np
import os
import zstd
parser = argparse.ArgumentParser()
parser.add_argument(
    "--base_dir", type=str, help="Base files location"
)
parser.add_argument(
    "--compare_dir", help="Comparison files location"
)
parser.add_argument(
    "--flatc_dir", default=None, help="flatc executable directory"
)
args = parser.parse_args()
flatc_file = 'flatc'
if args.flatc_dir is not None:
    flatc_file = os.path.join(args.flatc_dir, 'flatc')
try:
    import reinforcement_learning.messages.flatbuff.v2.EventBatch
except Exception as e:
    import pathlib
    import subprocess   

    script_dir = pathlib.Path(__file__).parent.absolute()
    input_dir = (
        pathlib.Path(script_dir).parents[1].joinpath("rlclientlib", "schema", "v2")
    )
    print(input_dir)

    input_files = " ".join([str(x) for x in input_dir.glob("*.fbs")])
    subprocess.run(
        f"{flatc_file} --python {input_files}", cwd=script_dir, shell=True, check=True
    )


# must be done after the above that generates the classes we're importing
from reinforcement_learning.messages.flatbuff.v2.EventBatch import EventBatch
from reinforcement_learning.messages.flatbuff.v2.Event import Event
from reinforcement_learning.messages.flatbuff.v2.EventEncoding import EventEncoding
from reinforcement_learning.messages.flatbuff.v2.LearningModeType import (
    LearningModeType,
)
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
from reinforcement_learning.messages.flatbuff.v2.EpisodeEvent import EpisodeEvent

from reinforcement_learning.messages.flatbuff.v2.FileHeader import *
from reinforcement_learning.messages.flatbuff.v2.JoinedEvent import *
from reinforcement_learning.messages.flatbuff.v2.JoinedPayload import *
from reinforcement_learning.messages.flatbuff.v2.CheckpointInfo import *
from reinforcement_learning.messages.flatbuff.v2.ProblemType import *


PREAMBLE_LENGTH = 8

class PreambleStreamReader:
    def __init__(self, file_name):
        self.file = open(file_name, "rb")

    def parse_preamble(self):
        buf = self.file.read(8)
        if buf == b"":
            return None

        reserved = buf[0]
        version = buf[1]
        msg_type = int.from_bytes(buf[2:4], "big")
        msg_size = int.from_bytes(buf[4:8], "big")
        return {
            "reserved": reserved,
            "version": version,
            "msg_type": msg_type,
            "msg_size": msg_size,
        }

    def messages(self):
        while True:
            header = self.parse_preamble()
            if header == None:
                break
            msg = self.file.read(header["msg_size"])

            batch = EventBatch.GetRootAsEventBatch(msg, 0)
            for i in range(0, batch.EventsLength()):
                evt = Event.GetRootAsEvent(batch.Events(i).PayloadAsNumpy(), 0)
                m = evt.Meta()

                payload = evt.PayloadAsNumpy()
                if m.Encoding() == EventEncoding.Zstd:
                    payload = zstd.decompress(evt.PayloadAsNumpy())

                if m.PayloadType() == PayloadType.CB:
                    yield (m.PayloadType(), CbEvent.GetRootAsCbEvent(payload, 0))
                elif m.PayloadType() == PayloadType.CCB or m.PayloadType() == PayloadType.Slates:
                    yield (m.PayloadType(), MultiSlotEvent.GetRootAsMultiSlotEvent(payload, 0))
                elif m.PayloadType() == PayloadType.Outcome:
                    yield (m.PayloadType(), OutcomeEvent.GetRootAsOutcomeEvent(payload, 0))
                elif m.PayloadType() == PayloadType.CA:
                    yield (m.PayloadType(), CaEvent.GetRootAsCaEvent(payload, 0))
                elif m.PayloadType() == PayloadType.DedupInfo:
                    yield (m.PayloadType(), DedupInfo.GetRootAsDedupInfo(payload, 0))
                elif m.PayloadType() == PayloadType.MultiStep:
                    yield (m.PayloadType(), MultiStepEvent.GetRootAsMultiStepEvent(payload, 0))
                elif m.PayloadType() == PayloadType.Episode:
                    yield (m.PayloadType(), EpisodeEvent.GetRootAsEpisodeEvent(payload, 0))
                else:
                    raise Exception("unknown payload type " + str(m.PayloadType()))

# Similar hack to the C# one due to limited binding codegen
def getString(table):
    off = table.Pos
    length = flatbuffers.encode.Get(
        flatbuffers.number_types.UOffsetTFlags.packer_type, table.Bytes, off
    )
    start = off + flatbuffers.number_types.UOffsetTFlags.bytewidth
    return bytes(table.Bytes[start : start + length])


def cast(table, tmp_type):
    tmp = tmp_type()
    tmp.Init(table.Bytes, table.Pos)
    return tmp

def compare_cb(base, compare):
    raise Exception('cb comparison is unsupported')

def compare_multislot(base, compare):
    raise Exception('multislot comparison is unsupported')

def compare_outcome(base, compare):
    if base.ValueType() != compare.ValueType() or base.IndexType() != compare.IndexType():
        raise Exception('outcome type is different')

    base_value = None
    compare_value = None
    if base.ValueType() == OutcomeValue.literal:
        base_value = getString(base.Value())
        compare_value = getString(compare.Value())
    elif base.ValueType() == OutcomeValue.numeric:
        base_value = cast(base.Value(), NumericOutcome).Value()
        compare_value = cast(compare.Value(), NumericOutcome).Value()
    if base_value != compare_value:
        raise Exception('outcome value is different')

    base_index = None
    compare_index = None
    if base.IndexType() == OutcomeValue.literal:
        base_index = getString(base.Index())
        compare_index = getString(compare.Index())
    elif base.IndexType() == OutcomeValue.numeric:
        base_index = cast(base.Index(), NumericIndex).Index()
        compare_index = cast(compare.Index(), NumericIndex).Index()
    if base_index != compare_index:
        raise Exception('outcome index is different')

    if base.ActionTaken() != compare.ActionTaken():
        raise Exception('outcome action_taken is different')

def compare_ca(base, compare):
    raise Exception('ca comparison is unsupported')

def compare_dedup(base, compare):
    raise Exception('dedup comparison is unsupported')

def compare_multistep(base, compare):
    # TODO: rl_sim uses boost::uuids::random_generator()() to generate eventIDs
    #       this means that eventIDs are not reproducible. Don't compare them for now
    #if base.EventId() != compare.EventId():
    #    raise Exception('Multistep event IDs do not match')
    #if base.PreviousId() != compare.PreviousId():
    #    raise Exception('Multistep event IDs do not match')
    if not np.array_equal(base.ActionIdsAsNumpy(), compare.ActionIdsAsNumpy()):
        raise Exception('Multistep ActionIDs do not match')
    if not np.array_equal(base.ContextAsNumpy(), compare.ContextAsNumpy()):
        raise Exception('Multistep Contexts do not match')
    if not np.array_equal(base.ProbabilitiesAsNumpy(), compare.ProbabilitiesAsNumpy()):
        raise Exception('Multistep Probabilities do not match')
    if base.ModelId() != compare.ModelId():
        raise Exception('Multistep model IDs do not match')
    if base.DeferredAction() != compare.DeferredAction():
        raise Exception('Multistep model IDs do not match')


def compare_episode(base, compare):
    if base.EpisodeId() != compare.EpisodeId():
        raise Exception('Episode IDs do not match')


def main():


    base_files = {f for f in os.listdir(args.base_dir) if os.path.isfile(os.path.join(args.base_dir, f))}
    compare_files = {f for f in os.listdir(args.compare_dir) if os.path.isfile(os.path.join(args.compare_dir, f))}

    if base_files != compare_files:
        raise Exception("base_dir and compare_dir must contain the same number of files with the same names")

    for filename in base_files:
        base_reader = PreambleStreamReader(os.path.join(args.base_dir, filename))
        compare_reader = PreambleStreamReader(os.path.join(args.compare_dir, filename))

        for base, compare in zip(base_reader.messages(), compare_reader.messages()):
            if base is None or compare is None:
                raise Exception("base and compare files do not contain the same number of events")
            if base[0] != compare[0]:
                raise Exception("base and compare types are no the same")
            if base[0] == PayloadType.CB:
                compare_cb(base[1], compare[1])
            elif base[0] == PayloadType.CCB or base[0] == PayloadType.Slates:
                compare_multislot(base[1], compare[1])
            elif base[0] == PayloadType.Outcome:
                compare_outcome(base[1], compare[1])
            elif base[0] == PayloadType.CA:
                compare_ca(base[1], compare[1])
            elif base[0] == PayloadType.DedupInfo:
                compare_dedup(base[1], compare[1])
            elif base[0] == PayloadType.MultiStep:
                compare_multistep(base[1], compare[1])
            elif base[0] == PayloadType.Episode:
                compare_episode(base[1], compare[1])
            else:
                raise Exception('Unknown type')


    print("Comparison completed. All events match")

        

if __name__ == "__main__":
    main()