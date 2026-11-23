from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

import flatbuffers

import compare_serialized_examples as compare
from reinforcement_learning.messages.flatbuff.v2 import IndexValue, NumericIndex, NumericOutcome, OutcomeEvent


def outcome(value, index=None):
    builder = flatbuffers.Builder(64)
    if isinstance(value, str):
        offset = builder.CreateString(value)
        value_type = compare.OutcomeValue.literal
    else:
        NumericOutcome.Start(builder)
        NumericOutcome.AddValue(builder, value)
        offset = NumericOutcome.End(builder)
        value_type = compare.OutcomeValue.numeric
    if isinstance(index, str):
        index_offset = builder.CreateString(index)
        index_type = IndexValue.IndexValue.literal
    elif index is not None:
        NumericIndex.Start(builder)
        NumericIndex.AddIndex(builder, index)
        index_offset = NumericIndex.End(builder)
        index_type = IndexValue.IndexValue.numeric
    OutcomeEvent.Start(builder)
    OutcomeEvent.AddValueType(builder, value_type)
    OutcomeEvent.AddValue(builder, offset)
    if index is not None:
        OutcomeEvent.AddIndexType(builder, index_type)
        OutcomeEvent.AddIndex(builder, index_offset)
    builder.Finish(OutcomeEvent.End(builder))
    return OutcomeEvent.OutcomeEvent.GetRootAs(builder.Output())


class SerializedComparisonTests(unittest.TestCase):
    def test_string_and_numeric_union_accessors_preserve_values(self):
        for value, index in (("literal reward", "slot"), (1.5, 3)):
            with self.subTest(value=value, index=index):
                compare.compare_outcome(outcome(value, index), outcome(value, index))

    def test_different_string_union_value_fails(self):
        with self.assertRaisesRegex(Exception, "outcome value is different"):
            compare.compare_outcome(outcome("first"), outcome("second"))
        with self.assertRaisesRegex(Exception, "outcome index is different"):
            compare.compare_outcome(outcome("same", "first"), outcome("same", "second"))

    def test_real_stream_fixtures_and_missing_events(self):
        base_dir = Path(__file__).resolve().parent / "base_files" / "input" / "multistep"
        with tempfile.TemporaryDirectory() as output:
            comparison_dir = Path(output)
            for source in base_dir.iterdir():
                shutil.copyfile(source, comparison_dir / source.name)
            argv = [
                "compare",
                "--base_dir",
                str(base_dir),
                "--compare_dir",
                str(comparison_dir),
            ]
            with patch.object(sys, "argv", argv):
                compare.main()
                (comparison_dir / "observation.fbs").write_bytes(b"")
                with self.assertRaisesRegex(Exception, "same number of events"):
                    compare.main()


if __name__ == "__main__":
    unittest.main()
