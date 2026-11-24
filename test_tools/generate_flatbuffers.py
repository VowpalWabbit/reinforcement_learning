"""Generate Python accessors without changing the wire schema's string unions."""

import importlib
import os
from pathlib import Path
import subprocess
import tempfile


def ensure_flatbuffer_bindings(output_dir):
    try:
        importlib.import_module(
            "reinforcement_learning.messages.flatbuff.v2.EventBatch"
        )
        return
    except ModuleNotFoundError as error:
        if not error.name or not (
            error.name == "reinforcement_learning"
            or error.name.startswith("reinforcement_learning.")
        ):
            raise

    schema_dir = Path(__file__).resolve().parents[1] / "rlclientlib" / "schema" / "v2"
    schemas = sorted(schema_dir.glob("*.fbs"))
    flatc = os.environ.get("FLATC", "flatc")
    with tempfile.TemporaryDirectory(prefix="rl-flatbuffers-") as binary_dir:
        # flatc 23.1.21 rejects string unions when parsing schemas for Python.
        # Binary reflection preserves those unions; Python's generic union
        # accessor supports the existing explicit string decoding.
        subprocess.run(
            [flatc, "--binary", "--schema", "-o", binary_dir, *map(str, schemas)],
            check=True,
        )
        subprocess.run(
            [
                flatc,
                "--python",
                "-o",
                str(output_dir),
                *map(str, sorted(Path(binary_dir).glob("*.bfbs"))),
            ],
            check=True,
        )
    importlib.invalidate_caches()
