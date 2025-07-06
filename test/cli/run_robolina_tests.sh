#!/bin/bash

# Check if the robolina tool path is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path-to-robolina-tool>"
    exit 1
fi

ROBOLINA_TOOL=$1
TEST_INPUT_DIR="testdirectory"
TEST_OUTPUT_DIR="test_results"

# Delete the test output directory if it already exists
if [ -d "$TEST_OUTPUT_DIR" ]; then
    rm -rf "$TEST_OUTPUT_DIR"
fi

# Create a directory for test results
mkdir -p "$TEST_OUTPUT_DIR"

mkdir -p "$TEST_OUTPUT_DIR/test_help1"
$ROBOLINA_TOOL -h > "$TEST_OUTPUT_DIR/test_help1/help1.txt" || { echo "Error: Failed to execute $ROBOLINA_TOOL for help1.txt"; exit 1; }
$ROBOLINA_TOOL --help > "$TEST_OUTPUT_DIR/test_help1/help2.txt" || { echo "Error: Failed to execute $ROBOLINA_TOOL for help2.txt"; exit 1; }

# Test 1: Replace "one two three" with "four five six"
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_default"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_default" "one two three" "four five six" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_default"; exit 1; }

# Test 2: Replace with --case-mode ignore
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_case_ignore"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_case_ignore" "One Two Three" "FOUR FIVE SIX" --case-mode ignore || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_case_ignore"; exit 1; }

# Test 3: Replace with --match-whole-word
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_whole_word"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_whole_word" "one" "ENO" --match-whole-word || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_whole_word"; exit 1; }

# Test 4: Replace recursively
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_recursive"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_recursive" "one two three" "four five six" --recursive || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_recursive"; exit 1; }

# Test 5: Replace with custom extensions
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_extensions"
$ROBOLINA_TOOL --extensions .notouch "$TEST_OUTPUT_DIR/test_extensions" "one two three" "four five six" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_extensions"; exit 1; }

# Test 6: Replace with --no-rename
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_no_rename"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_no_rename" "one two three" "four five six" --no-rename || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_no_rename"; exit 1; }

# Test 7: Replace with --dry-run
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_dry_run"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_dry_run" "one two three" "four five six" --dry-run > "$TEST_OUTPUT_DIR/test_dry_run.txt"  || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_dry_run"; exit 1; }

# Test 8: Replace using replacements file
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_replacements_file"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_replacements_file" --replacements-file "replacements.txt" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_replacements_file"; exit 1; }

# Test 9: Replace recursively verbose
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_verbose"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_verbose" "one two three" "four five six" --recursive -v > "$TEST_OUTPUT_DIR/test_verbose.txt" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_recursive"; exit 1; }

# Test 10: Replace with CString
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_cstring"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_cstring" "\n" "a\nb\nc" --case-mode ignore --no-rename || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_cstring"; exit 1; }

# Print completion message
echo "All CLI tests completed. Results are stored in $TEST_OUTPUT_DIR."
