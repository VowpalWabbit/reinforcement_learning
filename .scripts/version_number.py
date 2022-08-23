#!/usr/bin/env python3

import re
import sys
import subprocess


def debug_print(msg):
    print(f"[{sys.argv[0]}]: {msg}", file=sys.stderr)


git_describe = subprocess.check_output(
    ["git", "describe", "--tags", "--first-parent", "--long"], text=True
).strip()
debug_print("Output of 'git describe' is: " + git_describe)

r = re.compile(
    r"^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<tag>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?-(?P<commit>\d+)-g(?P<hash>[0-9a-fA-F]+)$"
)
m = r.match(git_describe)

major = m.group("major") or "No major version found"
minor = m.group("minor") or "No minor version found"
patch = m.group("patch") or "No patch version found"
tag = m.group("tag") or "No tag found"
commit = m.group("commit") or "No commit number found"
hash = m.group("hash") or "No git hash found"
debug_print(f"Major: {major}")
debug_print(f"Minor: {minor}")
debug_print(f"Patch: {patch}")
debug_print(f"Tag: {tag}")
debug_print(f"Commit: {commit}")
debug_print(f"Hash: {hash}")

if commit == "0":
    # Zero commits after tag means that this is the tagged commit
    # This is an official release, don't append the CI tag
    if m.group("tag") is None:
        print(f"{major}.{minor}.{patch}+{hash}")
    else:
        print(f"{major}.{minor}.{patch}-{tag}+{hash}")
else:
    # Non-official build
    # Append a "ci.[number]" tag
    if m.group("tag") is None:
        # Increment the patch number so that this build is versioned after previous official release
        print(f"{major}.{minor}.{int(patch)+1}-ci.{commit}+{hash}")
    else:
        # The most recent official release has a pre-release tag
        # Longer tags are always versioned after shorter tags, so no need to increment version number
        print(f"{major}.{minor}.{patch}-{tag}.ci.{commit}+{hash}")
