#!/bin/bash

# Set environment variable GH_WORKFLOW_LOGGING to output logging that Azure pipelines will interpret as a warning.
# Set environment variable WARNING_AS_ERROR to make the script exit with a non-zero code if issues are found.

if [[ "$1" != "check" ]] && [[ "$1" != "fix" ]]; then
    echo "Usage: \"$0 check\" or \"$0 fix\""
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/../../"
cd "$REPO_DIR"

for FILE in $(find . -type f -not -path './ext_libs/*' \( -name '*.cc' -o -name "*.h" \) ); do
    if [[ "$1" == "check" ]]; then
        diff $FILE <(clang-format $FILE);
        if [ $? -ne 0 ]; then
            ISSUE_FOUND="true"
            if [[ -v GH_WORKFLOW_LOGGING ]]; then
                echo "::warning:: Formatting issues found in $FILE"
            fi
        fi
    fi

    if [[ "$1" == "fix" ]]; then
        clang-format -i $FILE;
    fi
done

if [[ -v ISSUE_FOUND ]]; then
    echo -e "\nFormatting issues found:\n\tRun \"$0 fix\""
    if [[ -v WARNING_AS_ERROR ]]; then
        echo -e "\nTreating as failure because WARNING_AS_ERROR was set"
        exit 1
    fi
fi
