#!/bin/bash
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
