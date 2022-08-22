#!/usr/bin/env python3

import re
import subprocess

git = subprocess.check_output(["git", "describe", "--tags", "--first-parent", "--long"], text=True).strip()
# print("Output of 'git describe' is: " + git)

regex = re.compile(r'^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<tag>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?-(?P<commit>\d+)-g(?P<hash>[0-9a-fA-F]+)$')
m = regex.match(git)
major = m.group("major")
minor = m.group("minor")
patch = m.group("patch")
tag = m.group("tag")
commit = m.group("commit")
hash = m.group("hash")

if commit == "0":
    # Zero commits after tag means that this is the tagged commit
    # This is an official release, don't include the CI tag
    print(f"{major}.{minor}.{patch}{'-'+tag if tag else ''}+{hash}")
else:
    # Non-official build
    # Increment the patch number so that this build is versioned after previous official release
    print(f"{major}.{minor}.{int(patch)+1}-ci.{commit}+{hash}")
