from dataclasses import dataclass, field
from datetime import datetime
from typing import Union

import flatbuffers
import numpy as np

import reinforcement_learning.messages.flatbuff.v2.CbEvent as _CbEvent
import reinforcement_learning.messages.flatbuff.v2.Event as _Event
import reinforcement_learning.messages.flatbuff.v2.EventEncoding as _EventEncoding
import reinforcement_learning.messages.flatbuff.v2.IndexValue as _IndexValue
import reinforcement_learning.messages.flatbuff.v2.JoinedEvent as _JoinedEvent
import reinforcement_learning.messages.flatbuff.v2.LearningModeType as _LearningModeType
import reinforcement_learning.messages.flatbuff.v2.Metadata as _Metadata
import reinforcement_learning.messages.flatbuff.v2.MultiStepEvent as _MultiStepEvent
import reinforcement_learning.messages.flatbuff.v2.NumericIndex as _NumericIndex
import reinforcement_learning.messages.flatbuff.v2.NumericOutcome as _NumericOutcome
import reinforcement_learning.messages.flatbuff.v2.OutcomeEvent as _OutcomeEvent
import reinforcement_learning.messages.flatbuff.v2.OutcomeValue as _OutcomeValue
import reinforcement_learning.messages.flatbuff.v2.PayloadType as _PayloadType
import reinforcement_learning.messages.flatbuff.v2.TimeStamp as _TimeStamp


def EndVector(builder, size):
    from packaging import version
    if version.parse(flatbuffers.__version__).major >= 2:
        return builder.EndVector()
    else:
        return builder.EndVector(size)

def mk_offsets_vector(builder, arr, startFun):
    startFun(builder, len(arr))
    for i in reversed(range(len(arr))):
        builder.PrependUOffsetTRelative(arr[i])
    return EndVector(builder, len(arr))

def mk_bytes_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype='b'))

def mk_long_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype=np.int64))

def mk_float_vector(builder, arr):
    return builder.CreateNumpyVector(np.array(list(arr), dtype=np.float32))

def mk_timestamp(builder, dt):
    return _TimeStamp.CreateTimeStamp(builder,
                dt.year,
                dt.month,
                dt.day,
                dt.hour,
                dt.minute,
                dt.second,
                int(dt.microsecond))

@dataclass
class Metadata:
    payload_type: _PayloadType.PayloadType
    id: str
    client_time_utc: datetime
    encoding: _EventEncoding.EventEncoding
    pass_prob: float

    def to(self, builder):
        id = builder.CreateString(self.id)

        _Metadata.MetadataStart(builder)
        _Metadata.MetadataAddId(builder, id)
        _Metadata.MetadataAddClientTimeUtc(builder, mk_timestamp(builder, self.client_time_utc))
        _Metadata.MetadataAddPayloadType(builder, self.payload_type)
        _Metadata.MetadataAddEncoding(builder, self.encoding)
        _Metadata.MetadataAddPassProbability(builder=builder, passProbability = self.pass_prob)

        return _Metadata.MetadataEnd(builder)

@dataclass
class CbPayload:
    context: str
    actions: list
    probs: list
    deferred: bool
    model_id: str
    learning_mode: _LearningModeType.LearningModeType

    def to(self, builder):
        actions = mk_long_vector(builder, self.actions)
        ctx = mk_bytes_vector(builder, bytes(self.context, 'utf-8'))
        probs = mk_float_vector(builder, self.probs)
        model_id = builder.CreateString(self.model_id)

        _CbEvent.CbEventStart(builder)
        _CbEvent.CbEventAddDeferredAction(builder, self.deferred)
        _CbEvent.CbEventAddActionIds(builder, actions)
        _CbEvent.CbEventAddContext(builder, ctx)
        _CbEvent.CbEventAddProbabilities(builder, probs)
        _CbEvent.CbEventAddModelId(builder, model_id)
        _CbEvent.CbEventAddLearningMode(builder, self.learning_mode)
        return _CbEvent.CbEventEnd(builder)

@dataclass
class MultiStepPayload:
    id: str
    previous_id: str
    context: str
    actions: list
    probs: list
    deferred: bool
    model_id: str

    def to(self, builder):
        actions = mk_long_vector(builder, self.actions)
        ctx = mk_bytes_vector(builder, bytes(self.context, 'utf-8'))
        probs = mk_float_vector(builder, self.probs)
        model_id = builder.CreateString(self.model_id)
        id = builder.CreateString(self.id)
        previous_id = builder.CreateString(self.previous_id) if self.previous_id is not None else None

        _MultiStepEvent.MultiStepEventStart(builder)
        _MultiStepEvent.MultiStepEventAddEventId(builder, id)
        if previous_id is not None:
            _MultiStepEvent.MultiStepEventAddPreviousId(builder, previous_id)
        _MultiStepEvent.MultiStepEventAddDeferredAction(builder, self.deferred)
        _MultiStepEvent.MultiStepEventAddActionIds(builder, actions)
        _MultiStepEvent.MultiStepEventAddContext(builder, ctx)
        _MultiStepEvent.MultiStepEventAddProbabilities(builder, probs)
        _MultiStepEvent.MultiStepEventAddModelId(builder, model_id)
        return _MultiStepEvent.MultiStepEventEnd(builder)

@dataclass
class OutcomePayload:
    id: Union[str, int] = None
    value: Union[str, int] = None

    def to(self, builder):
        if isinstance(self.value, (int, float)):
            value_type = _OutcomeValue.OutcomeValue.numeric
        elif isinstance(self.value, str):
            value_type = _OutcomeValue.OutcomeValue.literal
        elif self.value is None:
            value_type = _OutcomeValue.OutcomeValue.NONE
        else:
            raise 'Unknown value type'

        if self.id is None:
            index_type = _IndexValue.IndexValue.NONE
        elif isinstance(self.id, (int, float)):
            index_type = _IndexValue.IndexValue.numeric
        elif isinstance(self.id, str):
            index_type = _IndexValue.IndexValue.literal
        else:
            raise 'Unknown index type'
        
        outcome = None
        if value_type == _OutcomeValue.OutcomeValue.numeric:
            _NumericOutcome.NumericOutcomeStart(builder)
            _NumericOutcome.NumericOutcomeAddValue(builder, self.value)
            outcome = _NumericOutcome.NumericOutcomeEnd(builder)
        elif value_type == _OutcomeValue.OutcomeValue.literal:
            outcome = builder.CreateString(self.value)

        index = None
        if index_type == _IndexValue.IndexValue.numeric:
            _NumericIndex.NumericIndexStart(builder)
            _NumericIndex.NumericIndexAddIndex(builder, self.id)
            index = _NumericIndex.NumericIndexEnd(builder)
        elif index_type == _OutcomeValue.OutcomeValue.literal:
            index = builder.CreateString(self.id)

        _OutcomeEvent.OutcomeEventStart(builder)
        _OutcomeEvent.OutcomeEventAddIndexType(builder, index_type)
        if index_type != _IndexValue.IndexValue.NONE:
            _OutcomeEvent.OutcomeEventAddIndex(builder, index)

        _OutcomeEvent.OutcomeEventAddValueType(builder, value_type)
        if value_type != _OutcomeValue.OutcomeValue.NONE:
            _OutcomeEvent.OutcomeEventAddValue(builder, outcome)
        _OutcomeEvent.OutcomeEventAddActionTaken(builder, value_type == _OutcomeValue.OutcomeValue.NONE)

        return _OutcomeEvent.OutcomeEventEnd(builder)

def serialize(payload):
    builder = flatbuffers.Builder(0)
    builder.Finish(payload.to(builder))
    return builder.Output()

def event_2_builder(meta, payload, builder):
    payload_str = serialize(payload)

    metadata_offset = meta.to(builder)
    payload_offset = mk_bytes_vector(builder, payload_str)

    _Event.EventStart(builder)
    _Event.EventAddMeta(builder, metadata_offset)
    _Event.EventAddPayload(builder, payload_offset)
    return _Event.EventEnd(builder)    

@dataclass
class CbEvent:
    id: str = 'id'
    client_time_utc: datetime = datetime(2021, 1, 1, 0, 0, 0)
    encoding: _EventEncoding.EventEncoding = _EventEncoding.EventEncoding.Identity
    pass_prob: float = 1.

    context: str = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
    actions: list = field(default_factory=lambda: [1, 2])
    probs: list = field(default_factory=lambda: [0.5, 0.5])
    deferred: bool = False
    model_id: str = 'model'
    learning_mode: _LearningModeType.LearningModeType = _LearningModeType.LearningModeType.Online 

    def to(self, builder):
        return event_2_builder(
            Metadata(
                payload_type = _PayloadType.PayloadType.CB,
                id = self.id,
                client_time_utc = self.client_time_utc,
                encoding = self.encoding,
                pass_prob = self.pass_prob),
            CbPayload(
                context = self.context,
                actions = self.actions,
                probs = self.probs,
                deferred = self.deferred,
                model_id = self.model_id,
                learning_mode = self.learning_mode),
            builder             
        )

    def serialize(self):
        return serialize(self)

@dataclass
class OutcomeEvent:
    primary_id: str = 'id'
    client_time_utc: datetime = datetime(2021, 1, 1, 0, 0, 0)
    encoding: _EventEncoding.EventEncoding = _EventEncoding.EventEncoding.Identity
    pass_prob: float = 1.

    secondary_id: Union[str, int] = None
    value: Union[str, int] = None

    def to(self, builder):
        return event_2_builder(
            Metadata(
                payload_type = _PayloadType.PayloadType.Outcome,
                id = self.primary_id,
                client_time_utc = self.client_time_utc,
                encoding = self.encoding,
                pass_prob = self.pass_prob),
            OutcomePayload(
                id = self.secondary_id,
                value = self.value),
            builder              
        )

    def serialize(self):
        return serialize(self)

@dataclass
class MultiStepEvent:
    episode_id: str = 'episode'
    client_time_utc: datetime = datetime(2021, 1, 1, 0, 0, 0)
    encoding: _EventEncoding.EventEncoding = _EventEncoding.EventEncoding.Identity
    pass_prob: float = 1.

    event_id: str = '0'
    previous_id: str = None
    context: str = """{"_multi":[{"a1":"f1"},{"a2":"f2"}]}"""
    actions: list = field(default_factory=lambda: [1, 2])
    probs: list = field(default_factory=lambda: [0.5, 0.5])
    deferred: bool = False
    model_id: str = 'model'

    def to(self, builder):
        return event_2_builder(
            Metadata(
                payload_type = _PayloadType.PayloadType.MultiStep,
                id = self.episode_id,
                client_time_utc = self.client_time_utc,
                encoding = self.encoding,
                pass_prob = self.pass_prob),
            MultiStepPayload(
                id = self.event_id,
                previous_id = self.previous_id,
                context = self.context,
                actions = self.actions,
                probs = self.probs,
                deferred = self.deferred,
                model_id = self.model_id),
            builder                
        )

    def serialize(self):
        return serialize(self)

@dataclass
class JoinedEvent:
    event: Union[CbEvent, MultiStepEvent, OutcomeEvent] = None
    timestamp: datetime = datetime(2021, 1, 1, 0, 0, 0) 

    def to(self, builder):
        payload_off = mk_bytes_vector(builder, self.event.serialize())
        _JoinedEvent.JoinedEventStart(builder)
        _JoinedEvent.JoinedEventAddEvent(builder, payload_off)
        _JoinedEvent.JoinedEventAddTimestamp(builder, mk_timestamp(builder, self.timestamp))
        return _JoinedEvent.JoinedEventEnd(builder)
