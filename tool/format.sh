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

TOP_DIR=$PWD

# Define the directories to process
DIRECTORIES=("$TOP_DIR/src")

# Check if a .clang-format file exists in the current directory. If not, create one.
if [ ! -f "$TOP_DIR/.clang-format" ]; then
    echo "No .clang-format file found in $TOP_DIR. Creating a temporary one..."
    echo "BasedOnStyle: LLVM" > "$TOP_DIR/.clang-format"
    echo "IndentWidth: 4" >> "$TOP_DIR/.clang-format"
    echo "AccessModifierOffset: -4" >> "$TOP_DIR/.clang-format"
    echo "IndentPPDirectives: BeforeHash" >> "$TOP_DIR/.clang-format"
    echo "IndentAccessModifiers: false" >> "$TOP_DIR/.clang-format"
fi

# Iterate through the specified directories and format the files
for dir in "${DIRECTORIES[@]}"; do
    echo "Formatting directory: $dir"
    FORMATTED_FILES=()

    # Find all .cpp, .c, .h files in the directory
    FILES=$(find "$dir" -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -print0)

    # Check if any files were found
    if [ -z "$FILES" ]; then
        echo "No files found with supported extensions in $dir."
        continue
    fi

    # Use a while loop to process each file (using null characters as separators)
    while IFS= read -r -d '' file; do
        clang-format -style=file -i "$file"
        FORMATTED_FILES+=("$file")
    done < <(find "$dir" -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -print0)

    # Output the list of formatted files
    if [ ${#FORMATTED_FILES[@]} -gt 0 ]; then
        printf "\nFormatted files in %s:\n" "$dir"
        printf "%s\n" "${FORMATTED_FILES[@]}"
    fi
done

echo "Formatting completed."
