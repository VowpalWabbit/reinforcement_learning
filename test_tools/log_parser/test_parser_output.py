import contextlib
import io
from pathlib import Path
import struct
import subprocess
import sys
import unittest
from unittest.mock import patch

import parser


FIXTURES = (
    Path(__file__).resolve().parents[2]
    / "external_parser"
    / "unit_tests"
    / "test_files"
)
JOINED_FILE = FIXTURES / "valid_joined_logs" / "cb_simple.log"
PREAMBLE_FILE = FIXTURES / "fb_events" / "cb_v2.fb"


def capture(function, *args, **kwargs):
    with contextlib.redirect_stdout(io.StringIO()) as output:
        function(*args, **kwargs)
    return output.getvalue()


def frame(kind, payload):
    return struct.pack("<II", kind, len(payload)) + payload + bytes(len(payload) % 8)


class ParserOutputTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.joined = JOINED_FILE.read_bytes()
        cls.preamble = PREAMBLE_FILE.read_bytes()
        reader = parser.JoinedLogStreamReader(cls.joined)
        cls.payloads = {}
        while True:
            msg = reader.read_message()
            if msg is None or msg[0] == parser.MSG_TYPE_EOF:
                break
            cls.payloads.setdefault(msg[0], msg[1])

    def test_joined_summary_counts_checkpoint_segments_without_event_decoding(self):
        regular = self.payloads[parser.MSG_TYPE_REGULAR]
        checkpoint = self.payloads[parser.MSG_TYPE_CHECKPOINT]
        buf = (
            b"VWFB"
            + struct.pack("<I", 1)
            + frame(parser.MSG_TYPE_CHECKPOINT, checkpoint)
            + frame(parser.MSG_TYPE_REGULAR, regular) * 2
            + frame(parser.MSG_TYPE_CHECKPOINT, checkpoint)
            + frame(parser.MSG_TYPE_REGULAR, regular)
            + struct.pack("<I", parser.MSG_TYPE_EOF)
        )
        with patch.object(parser, "dump_event") as dump_event, patch.object(
            parser.zstd, "decompress"
        ) as decompress:
            output = capture(
                parser.dump_joined_log_file, "bounded.log", buf, False, summary=True
            )
        self.assertIn("messages before:0 data before:0 bytes", output)
        self.assertIn(f"messages before:2 data before:{2 * len(regular)} bytes", output)
        self.assertIn(f"remaining messages: 1 size: {len(regular)} bytes", output)
        self.assertEqual(output.count("checkpoint len:"), 2)
        dump_event.assert_not_called()
        decompress.assert_not_called()
        # A file without checkpoints still reports its final segment.
        output = capture(
            parser.dump_joined_log_file,
            "bounded.log",
            frame(parser.MSG_TYPE_REGULAR, regular),
            False,
            summary=True,
        )
        self.assertIn(f"remaining messages: 1 size: {len(regular)} bytes", output)

    def test_short_joined_and_preamble_suppress_event_payloads(self):
        with patch.object(parser, "dump_event") as dump_event, patch.object(
            parser.zstd, "decompress"
        ) as decompress:
            joined = capture(
                parser.dump_joined_log_file,
                "bounded.log",
                self.joined,
                False,
                short=True,
            )
            compressed = (
                FIXTURES / "valid_joined_logs" / "cb_dedup_compressed.log"
            ).read_bytes()
            for mode in ({"short": True}, {"summary": True}):
                capture(
                    parser.dump_joined_log_file,
                    "compressed.log",
                    compressed,
                    False,
                    **mode,
                )
            preamble = capture(
                parser.dump_preamble_file, "bounded.fb", self.preamble, short=True
            )
        self.assertIn("regular-msg len:", joined)
        self.assertIn("checkpoint len:", joined)
        self.assertNotIn("joined-batch events:", joined)
        self.assertIn("event-batch evt-count:", preamble)
        self.assertNotIn("context:", joined + preamble)
        dump_event.assert_not_called()
        decompress.assert_not_called()

    def test_preamble_summary_counts_concatenated_batches(self):
        first = parser.parse_preamble(self.preamble)
        batch = self.preamble[: parser.PREAMBLE_LENGTH + first["msg_size"]]
        events = parser.EventBatch.GetRootAsEventBatch(
            batch[parser.PREAMBLE_LENGTH :], 0
        ).EventsLength()
        with patch.object(parser, "dump_event") as dump_event:
            output = capture(
                parser.dump_preamble_file, "bounded.fb", batch * 2, summary=True
            )
        self.assertEqual(
            output,
            f"preamble file:bounded.fb batches:2 events:{2 * events} "
            f"data:{2 * first['msg_size']} bytes\n",
        )
        dump_event.assert_not_called()

    def test_modes_do_not_change_subsequent_default_output_or_records(self):
        fixture = JOINED_FILE
        original = capture(parser.dump_file, fixture)
        records = repr(list(parser.get_records(fixture)))
        capture(parser.dump_file, fixture, summary=True)
        capture(parser.dump_file, fixture, short=True)
        self.assertEqual(capture(parser.dump_file, fixture), original)
        self.assertEqual(repr(list(parser.get_records(fixture))), records)
        self.assertIn("joined-batch events:", original)
        self.assertIn("reward function type is:", original)
        self.assertIn(
            "events:0",
            capture(parser.dump_preamble_file, "empty.fb", b"", summary=True),
        )

    def test_cli_multiple_files_and_exclusive_output_modes(self):
        command = [sys.executable, str(Path(parser.__file__)), "--summary"]
        result = subprocess.run(
            command + [str(JOINED_FILE), str(PREAMBLE_FILE)],
            check=True,
            capture_output=True,
            text=True,
        )
        self.assertIn("finished reading remaining messages:", result.stdout)
        self.assertIn("preamble file:", result.stdout)
        self.assertNotIn("context:", result.stdout)
        result = subprocess.run(
            command + ["--verbose", str(JOINED_FILE)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("not allowed with argument", result.stderr)


if __name__ == "__main__":
    unittest.main()
