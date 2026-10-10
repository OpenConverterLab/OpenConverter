#!/usr/bin/env python3
# Copyright 2026 Jack Lau
# Email: jacklau1222gm@gmail.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Fail when a source file is missing the project's Apache-2.0 license header.

Used as a pre-commit hook (see .github/pre-commit/config.yaml). Only source
files are checked; documentation and third-party files are out of scope.
"""
import sys
from pathlib import Path

# a file passes when the first lines contain both markers
MARKERS = (
    "Copyright 20",
    "Licensed under the Apache License, Version 2.0",
)
HEAD_LINES = 25


def has_header(path: Path) -> bool:
    try:
        lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
    except OSError:
        return False
    head = "\n".join(lines[:HEAD_LINES])
    return all(marker in head for marker in MARKERS)


def main(argv):
    missing = [f for f in argv if Path(f).is_file() and not has_header(Path(f))]
    if missing:
        print("Missing Apache-2.0 license header (see .augment-guidelines):")
        for f in missing:
            print(f"  {f}")
        print("\nAdd the header documented in .augment-guidelines to each file above.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
