@echo off
REM Check if the robolina tool path is provided
IF "%~1"=="" (
    echo Usage: %~nx0 ^<path-to-robolina-tool^>
    exit /b 1
)

rem The default encoding for command prompt is Windows-1252.
rem Change the code page (chcp command) to 65001 (UTF-8) first and then run your command.
chcp 65001

set "ROBOLINA_TOOL=%~1"
set "TEST_INPUT_DIR=testdirectory"
set "TEST_OUTPUT_DIR=test_results"

REM Delete the test output directory if it already exists
IF EXIST "%TEST_OUTPUT_DIR%" rmdir /s /q "%TEST_OUTPUT_DIR%"

REM Create a directory for test results
mkdir "%TEST_OUTPUT_DIR%"

mkdir "%TEST_OUTPUT_DIR%\test_help1"
%ROBOLINA_TOOL% -h > "%TEST_OUTPUT_DIR%\test_help1\help1.txt" || goto :error
%ROBOLINA_TOOL% --help > "%TEST_OUTPUT_DIR%\test_help1\help2.txt" || goto :error

REM Test 1: Replace "one two three" with "four five six"
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_default"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_default" "one two three" "four five six" || goto :error

REM Test 2: Replace with --case-mode ignore
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_case_ignore"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_case_ignore" "One Two Three" "FOUR FIVE SIX" --case-mode ignore || goto :error

REM Test 3: Replace with --match-whole-word
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_whole_word"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_whole_word" "one" "ENO" --match-whole-word || goto :error

REM Test 4: Replace recursively
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_recursive"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_recursive" "one two three" "four five six" --recursive || goto :error

REM Test 5: Replace with custom extensions
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_extensions"
%ROBOLINA_TOOL% --extensions .notouch "%TEST_OUTPUT_DIR%\test_extensions" "one two three" "four five six" || goto :error

REM Test 6: Replace with --no-rename
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_no_rename"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_no_rename" "one two three" "four five six" --no-rename || goto :error

REM Test 7: Replace with --dry-run
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_dry_run"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_dry_run" "one two three" "four five six" --dry-run > "%TEST_OUTPUT_DIR%\test_dry_run.txt" || goto :error

REM Test 8: Replace using replacements file
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_replacements_file"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_replacements_file" --replacements-file "replacements.txt" || goto :error

REM Test 9: Replace recursively verbose
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_verbose"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_verbose" "one two three" "four five six" --recursive -v > "%TEST_OUTPUT_DIR%\test_verbose.txt" || goto :error

REM Test 10: Replace with CString
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_cstring"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_cstring" "\r\n" "a\r\nb\r\nc" --case-mode ignore || goto :error

REM Test 11: Replace one file
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_one_file"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_one_file\testfile1_OneTwoThree.txt" "one two three" "four five six" || goto :error

REM Test 12: Replace using replacements file and replacement arguments
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_replacements_file_with_args"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_replacements_file_with_args" "kebab" "shish kebap" --replacements-file "replacements.txt" || goto :error

REM Test 13: Replace using replacements file and replacement arguments (non-ascii path)
rem The default encoding for command prompt is Windows-1252.
rem Change the code page (chcp command) to 65001 (UTF-8) first and then run your command.
chcp 65001
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_non_ascii_path_ä"
REM chcp 1252
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_non_ascii_path_ä" "one two three" "four five six" || goto :error

REM Test 14: Replace using replacements file with short syntax
xcopy /E /I /Q "%TEST_INPUT_DIR%" "%TEST_OUTPUT_DIR%\test_replacements_file_short_syntax"
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_replacements_file_short_syntax" --replacements-file "replacements_short_syntax.txt" || goto :error

REM Test Error: Missing required positional arguments
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" 2> "%TEST_OUTPUT_DIR%\bad_missing_args1.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_args1.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" 2> "%TEST_OUTPUT_DIR%\bad_missing_args2.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_args2.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% 2> "%TEST_OUTPUT_DIR%\bad_missing_args3.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_args3.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" --replacements-file "replacements.txt" 2> "%TEST_OUTPUT_DIR%\bad_missing_args4.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_args4.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% --replacements-file "replacements.txt" 2> "%TEST_OUTPUT_DIR%\bad_missing_args5.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_args5.txt, got: %ERRORLEVEL%
    exit /b 1
)

REM Test Error: Too many positional arguments
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" "seven eight nine" 2> "%TEST_OUTPUT_DIR%\bad_too_many_args1.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_too_many_args1.txt, got: %ERRORLEVEL%
    exit /b 1
)

REM Test Error: Unknown option
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --lorum 2> "%TEST_OUTPUT_DIR%\bad_unknown_args1.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_unknown_args1.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" -lorum 2> "%TEST_OUTPUT_DIR%\bad_unknown_args2.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_unknown_args2.txt, got: %ERRORLEVEL%
    exit /b 1
)

REM Test Error: Missing value
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --case-mode 2> "%TEST_OUTPUT_DIR%\bad_missing_value1.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_value1.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --extensions 2> "%TEST_OUTPUT_DIR%\bad_missing_value2.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_value2.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --replacements-file 2> "%TEST_OUTPUT_DIR%\bad_missing_value3.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_missing_value3.txt, got: %ERRORLEVEL%
    exit /b 1
)

REM Test Error: Bad value
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --case-mode "kebab" 2> "%TEST_OUTPUT_DIR%\bad_value1.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value1.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --extensions "" 2> "%TEST_OUTPUT_DIR%\bad_value2.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value2.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "one two three" "four five six" --replacements-file "notthere" 2> "%TEST_OUTPUT_DIR%\bad_value3.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value3.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\notthere" "one two three" "four five six" 2> "%TEST_OUTPUT_DIR%\bad_value4.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value4.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\dummy" "" "four five six" 2> "%TEST_OUTPUT_DIR%\bad_value5.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value5.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_default" "one two three" "four five six" --replacements-file "replacements_bad1.txt" 2> "%TEST_OUTPUT_DIR%\bad_value6.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value6.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_default" "one two three" "four five six" --replacements-file "replacements_bad2.txt" 2> "%TEST_OUTPUT_DIR%\bad_value7.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value7.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_default" "one two three" "four five six" --replacements-file "replacements_bad3.txt" 2> "%TEST_OUTPUT_DIR%\bad_value8.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value8.txt, got: %ERRORLEVEL%
    exit /b 1
)
%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_default" "one two three" "four five six" --replacements-file "replacements_bad4.txt" 2> "%TEST_OUTPUT_DIR%\bad_value9.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value9.txt, got: %ERRORLEVEL%
    exit /b 1
)

%ROBOLINA_TOOL% "%TEST_OUTPUT_DIR%\test_default" "one two three" "four five six" --replacements-file "replacements_bad5.txt" 2> "%TEST_OUTPUT_DIR%\bad_value10.txt"
IF NOT ERRORLEVEL 1 (
    echo Error: Expected exit code 1 for bad_value10.txt, got: %ERRORLEVEL%
    exit /b 1
)

echo All tests completed successfully.
exit /b 0

:error
echo Error: Execution of %ROBOLINA_TOOL% failed.
exit /b 1
