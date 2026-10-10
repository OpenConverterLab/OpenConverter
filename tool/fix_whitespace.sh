#!/bin/bash
# Copyright 2025 Jack Lau
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

# Fix trailing whitespace and ensure single newline at end of file

# Get list of staged files (C++ and header files)
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp|cc|cxx)$')

if [ -z "$files" ]; then
    echo "No C++ files to fix"
    exit 0
fi

echo "Fixing whitespace issues in files..."
echo ""

for file in $files; do
    if [ -f "$file" ]; then
        echo "  $file"

        # Remove trailing whitespace from each line
        sed -i '' 's/[[:space:]]*$//' "$file"

        # Ensure file ends with exactly one newline
        # This perl command ensures the file ends with exactly one newline
        perl -pi -e 'BEGIN{undef $/;} s/\s*\z/\n/' "$file"
    fi
done

echo ""
echo "Done! Fixed whitespace in $(echo "$files" | wc -l | tr -d ' ') files"
echo ""
echo "⚠️  Please review the changes and add them manually:"
echo "   git add <files>"
echo "   or: git add -u"
