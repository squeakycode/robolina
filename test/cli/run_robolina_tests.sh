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
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_cstring" "\n" "a\nb\nc" --case-mode ignore || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_cstring"; exit 1; }

# Test 11: Replace one file
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_one_file"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_one_file/testfile1_OneTwoThree.txt" "one two three" "four five six" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_one_file"; exit 1; }

# Test 12: Replace using replacements file and replacement arguments
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_replacements_file_with_args"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_replacements_file_with_args" "kebab" "shish kebap" --replacements-file "replacements.txt" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_replacements_file_with_args"; exit 1; }

# Test 13: Replace using replacements file and replacement arguments
cp -R "$TEST_INPUT_DIR" "$TEST_OUTPUT_DIR/test_non_ascii_path_ä"
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_non_ascii_path_ä" "one two three" "four five six" || { echo "Error: Failed to execute $ROBOLINA_TOOL for test_non_ascii_path_ä"; exit 1; }

# Test Error: Missing required positional arguments
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" 2> "$TEST_OUTPUT_DIR/bad_missing_args1.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_args1.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" 2> "$TEST_OUTPUT_DIR/bad_missing_args2.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_args2.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL 2> "$TEST_OUTPUT_DIR/bad_missing_args3.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_args3.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" --replacements-file "replacements.txt" 2> "$TEST_OUTPUT_DIR/bad_missing_args4.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_args4.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL --replacements-file "replacements.txt" 2> "$TEST_OUTPUT_DIR/bad_missing_args5.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_args5.txt, got $?"; exit 1;
fi

# Test Error: Too many positional arguments
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" "seven eight nine" 2> "$TEST_OUTPUT_DIR/bad_too_many_args1.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_too_many_args1.txt, got $?"; exit 1;
fi

# Test Error: Unknown option
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --lorum 2> "$TEST_OUTPUT_DIR/bad_unknown_args1.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_unknown_args1.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" -lorum 2> "$TEST_OUTPUT_DIR/bad_unknown_args2.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_unknown_args2.txt, got $?"; exit 1;
fi

# Test Error: Missing value
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --case-mode 2> "$TEST_OUTPUT_DIR/bad_missing_value1.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_value1.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --extensions 2> "$TEST_OUTPUT_DIR/bad_missing_value2.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_value2.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --replacements-file 2> "$TEST_OUTPUT_DIR/bad_missing_value3.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_missing_value3.txt, got $?"; exit 1;
fi

# Test Error: Bad value
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --case-mode "kebab" 2> "$TEST_OUTPUT_DIR/bad_value1.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value1.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --extensions "" 2> "$TEST_OUTPUT_DIR/bad_value2.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value2.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "one two three" "four five six" --replacements-file "notthere" 2> "$TEST_OUTPUT_DIR/bad_value3.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value3.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/notthere" "one two three" "four five six" 2> "$TEST_OUTPUT_DIR/bad_value4.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value4.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/dummy" "" "four five six" 2> "$TEST_OUTPUT_DIR/bad_value5.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value5.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_default" "one two three" "four five six" --replacements-file "replacements_bad1.txt" 2> "$TEST_OUTPUT_DIR/bad_value6.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value6.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_default" "one two three" "four five six" --replacements-file "replacements_bad2.txt"  2> "$TEST_OUTPUT_DIR/bad_value7.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value7.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_default" "one two three" "four five six" --replacements-file "replacements_bad3.txt" 2> "$TEST_OUTPUT_DIR/bad_value8.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value8.txt, got $?"; exit 1;
fi
$ROBOLINA_TOOL "$TEST_OUTPUT_DIR/test_default" "one two three" "four five six" --replacements-file "replacements_bad4.txt" 2> "$TEST_OUTPUT_DIR/bad_value9.txt"
if [ $? -ne 1 ]; then
    echo "Error: Expected exit code 1 for bad_value9.txt, got $?"; exit 1;
fi

# Print completion message
echo "All CLI tests completed. Results are stored in $TEST_OUTPUT_DIR."
