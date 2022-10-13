import struct
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

from reinforcement_learning.messages.flatbuff.v2.FileHeader import *
from reinforcement_learning.messages.flatbuff.v2.JoinedEvent import *
from reinforcement_learning.messages.flatbuff.v2.JoinedPayload import *
from reinforcement_learning.messages.flatbuff.v2.CheckpointInfo import *
from reinforcement_learning.messages.flatbuff.v2.RewardFunctionType import *


class Base64Tensor:
    @staticmethod
    def parse(line):
        import numpy as np
        import base64

        prefix, value = line.split(";")
        # many hacks
        shape = struct.unpack("4Q", base64.b64decode(prefix))
        shape = shape[1:]
        return np.array(
            struct.unpack("%df" % np.prod(shape), base64.b64decode(value))
        ).reshape(shape)

    @staticmethod
    def parse_dict(context):
        return dict(map(lambda kv: (kv[0], Base64Tensor.parse(kv[1])), context.items()))


class CbDsjsonParser:
    @staticmethod
    def parse(line):
        import json

        obj = json.loads(line)
        return {
            "features": Base64Tensor.parse_dict(obj["c"]),
            "label": obj["_labelIndex"],
            "cost": obj["_label_cost"],
        }


class CbDictParser:
    @staticmethod
    def parse(obj):
        import numpy as np
        import json

        # features is a json payload wrapped in a bytearray
        features = json.loads(obj[0].decode("utf-8"))
        label = obj[1]
        # I think the logs provide 1-indexed values while the train function is expecting 0-indexed
        label = label - 1
        from torch import tensor

        return {
            "features": Base64Tensor.parse_dict(features),
            "label": label,
            "cost": obj[2],
        }


MSG_TYPE_HEADER = 0x55555555
MSG_TYPE_CHECKPOINT = 0x11111111
MSG_TYPE_REGULAR = 0xFFFFFFFF
MSG_TYPE_EOF = 0xAAAAAAAA
# mostly ripped from the flatbuf parser
class JoinedLogStreamReader:
    def __init__(self, buf):
        self.buf = buf
        self.offset = 0
        self.headers = dict()
        self.parse_header()
        self.read_file_header()

    def parse_header(self):
        if self.buf[0:4] != b"VWFB":
            raise Exception("Invalid file magic")

        self.version = struct.unpack("I", self.buf[4:8])[0]
        if self.version != 1:
            raise Exception(f"Unsuported file version {self.version}")
        self.offset = 8

    def read(self, size):
        if size == 0:
            return bytearray([])
        data = self.buf[self.offset : self.offset + size]
        self.offset += size
        return data

    def read_message(self):
        kind = struct.unpack("I", self.read(4))[0]
        length = struct.unpack("I", self.read(4))[0]
        payload = self.read(length)
        # discard padding
        self.read(length % 8)
        return (kind, payload)

    def read_file_header(self):
        msg = self.read_message()
        if msg[0] != MSG_TYPE_HEADER:
            raise f"Missing file header, found message of type {msg[0]} instead"

        header = FileHeader.GetRootAsFileHeader(msg[1], 0)
        for i in range(header.PropertiesLength()):
            p = header.Properties(i)
            self.headers[p.Key().decode("utf-8")] = p.Value().decode("utf-8")

    def checkpoint_info(self):
        msg = self.read_message()
        if msg[0] != MSG_TYPE_CHECKPOINT:
            raise f"Missing checkpoint info, found message type of {msg[0]} instead"
        return CheckpointInfo.GetRootAsCheckpointInfo(msg[1], 0)

    def messages(self):
        while True:
            msg = self.read_message()
            if msg[0] == MSG_TYPE_EOF:
                break
            if msg[0] != MSG_TYPE_REGULAR:
                raise f"Expected Regular message type, found {msg[0]} instead"
            yield JoinedPayload.GetRootAsJoinedPayload(msg[1], 0)
        return None


# partially ripped from the flatbuf parser
class VWFlatbufferParser:
    # data should be a bytearray
    def __init__(self, data):
        self.reader = JoinedLogStreamReader(data)
        self.current_id = None
        self.checkpoint_info = self.reader.checkpoint_info()

        self.gen = self.read_event_series()

    def __iter__(self):
        return self.gen

    def read_as_string(self, table):
        off = table.Pos
        length = flatbuffers.encode.Get(
            flatbuffers.number_types.UOffsetTFlags.packer_type, table.Bytes, off
        )
        start = off + flatbuffers.number_types.UOffsetTFlags.bytewidth
        return bytes(table.Bytes[start : start + length])

    def cast(self, table, tmp_type):
        tmp = tmp_type()
        tmp.Init(table.Bytes, table.Pos)
        return tmp

    def parse_cb(self, payload):
        evt = CbEvent.GetRootAsCbEvent(payload, 0)
        payload = evt.ContextAsNumpy()
        label = evt.ActionIds(0)
        return bytearray(payload), label

    def parse_outcome(self, payload):
        evt = OutcomeEvent.GetRootAsOutcomeEvent(payload, 0)

        value = evt.Value()
        if evt.ValueType() == OutcomeValue.literal:
            value = self.read_as_string(value)
        elif evt.ValueType() == OutcomeValue.numeric:
            value = self.cast(value, NumericOutcome).Value()

        index = None
        if evt.ActionTaken() is True:
            index = evt.Index()
            if evt.IndexType() == OutcomeValue.literal:
                # parse_cb only contains numeric index types, so this should also be a numeric index type
                raise Exception("literal index types are unsupported")
            elif evt.IndexType() == OutcomeValue.numeric:
                index = self.cast(index, NumericIndex).Index()

        return value, index

    def apply_reward_fn(self, rewards):
        reward_fn_type = self.checkpoint_info.RewardFunctionType()
        if len(rewards) == 0:
            return self.checkpoint_info.DefaultReward()
        elif reward_fn_type == RewardFunctionType.Earliest:
            return rewards[0]
        elif reward_fn_type == RewardFunctionType.Average:
            return sum(rewards) / len(rewards)
        elif reward_fn_type == RewardFunctionType.Median:
            from statistics import median

            return median(rewards)
        elif reward_fn_type == RewardFunctionType.Sum:
            return sum(rewards)
        elif reward_fn_type == RewardFunctionType.Min:
            return min(rewards)
        elif reward_fn_type == RewardFunctionType.Max:
            return max(rewards)
        else:
            raise Exception("Unknown reward function type")

    # reads all events associated with the next event_id
    def read_event_series(self, timestamp=None):
        for msg in self.reader.messages():
            # only support CB for now
            label = None
            rewards = []
            current_id = None
            current_payload = None

            # One thing to note, the chosen action is either encoded in the ActionIds array
            # in the CB payload as the first element (if DeferredAction == False)
            # or its in the outcomes payload as the ActionTaken if DeferredAction == True
            for i in range(msg.EventsLength()):
                joined_event = msg.Events(i).EventAsNumpy()
                evt = Event.GetRootAsEvent(joined_event, 0)
                m = evt.Meta()
                event_payload = evt.PayloadAsNumpy()

                if m.Encoding() == EventEncoding.Zstd:
                    event_payload = zstd.decompress(event_payload)

                if m.PayloadType() == PayloadType.CB:
                    # ew gross
                    if label is not None:
                        cost = -1 * self.apply_reward_fn(rewards)
                        yield current_payload, label, cost
                    current_id = m.Id()
                    current_payload, label = self.parse_cb(event_payload)
                elif m.PayloadType() == PayloadType.Outcome:
                    tmpreward, tmplabel = self.parse_outcome(event_payload)
                    if tmpreward is not None:
                        rewards.append(tmpreward)
                    if tmplabel is not None:
                        label = tmplabel
                elif m.PayloadType() == PayloadType.DedupInfo:
                    raise Exception("Not Implemented")
                    continue
                else:
                    raise Exception("unknown payload type")

        return current_payload, label, cost
