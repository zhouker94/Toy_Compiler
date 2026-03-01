#!/bin/bash

# Ensure we are in the root directory where bin/ and examples/ are located
cd "$(dirname "$0")/.." || exit 1

BIN="./bin/minicc"
EXAMPLES_DIR="./examples"
PASSED=0
FAILED=0

if [ ! -f "$BIN" ]; then
    echo "Error: Binary $BIN not found. Please run 'make' first."
    exit 1
fi

echo "Running miniCC example tests..."
echo "==============================="

for file in "$EXAMPLES_DIR"/*.c; do
    filename=$(basename "$file")
    base_name="${filename%.*}"

    executable_path="$(dirname "$BIN")/$base_name"

    echo -n "Testing $filename... "

    # Run the compiler, suppress output
    "$BIN" "$file" > /dev/null 2>&1
    EXIT_CODE=$?

    # 1. Check compilation result
    if [[ "$filename" == "error.c" ]]; then
        if [ $EXIT_CODE -ne 0 ]; then
            echo "[PASSED] (Compilation failed as expected)"
            PASSED=$((PASSED+1))
        else
            echo "[FAILED] (Compilation succeeded but was expected to fail)"
            FAILED=$((FAILED+1))
        fi
        continue
    else
        if [ $EXIT_CODE -ne 0 ]; then
            echo "[FAILED] (Compilation failed with exit code: $EXIT_CODE)"
            FAILED=$((FAILED+1))
            continue
        fi
    fi

    # 2. Correctness check for successfully compiled files
    # Use a portable sed command to extract the expected exit code. Handles optional colon.
    expected_exit_code=$(sed -n -E 's/.*expected:?[[:space:]]+([0-9]+).*/\1/p' "$file" | head -n 1)
    if [ -z "$expected_exit_code" ]; then
        echo "[PASSED] (Compiled successfully, no correctness check)"
        PASSED=$((PASSED+1))
        continue
    fi

    "$executable_path"
    actual_exit_code=$?

    if [ "$actual_exit_code" -eq "$expected_exit_code" ]; then
        echo "[PASSED] (Correctness check passed, exit code: $actual_exit_code)"
        PASSED=$((PASSED+1))
    else
        echo "[FAILED] (Correctness check failed. Expected: $expected_exit_code, Got: $actual_exit_code)"
        FAILED=$((FAILED+1))
    fi
done

echo "==============================="
echo "Summary: $PASSED passed, $FAILED failed."

if [ $FAILED -gt 0 ]; then
    exit 1
fi
