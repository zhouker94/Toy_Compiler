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
    echo -n "Testing $filename... "
    
    # Run the compiler, suppress output
    "$BIN" "$file" > /dev/null 2>&1
    EXIT_CODE=$?

    # Logic: error.c should return non-zero, others should return 0
    if [[ "$filename" == "error.c" ]]; then
        if [ $EXIT_CODE -ne 0 ]; then
            echo "[PASSED] (Expected failure)"
            PASSED=$((PASSED+1))
        else
            echo "[FAILED] (Expected error but parsing succeeded)"
            FAILED=$((FAILED+1))
        fi
    else
        if [ $EXIT_CODE -eq 0 ]; then
            echo "[PASSED]"
            PASSED=$((PASSED+1))
        else
            echo "[FAILED] (Exit code: $EXIT_CODE)"
            FAILED=$((FAILED+1))
        fi
    fi
done

echo "==============================="
echo "Summary: $PASSED passed, $FAILED failed."

if [ $FAILED -gt 0 ]; then
    exit 1
fi
