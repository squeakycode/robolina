# Robolina - Find and Replace in Files Preserving Case

## Purpose

This is a header-only code library and command-line tool designed for performing 
bulk find-and-replace operations in source code files, including filenames. The 
replacement text can be applied while preserving the casing of the original text.

| Example        | Casing            |
|--------------- |------------------|
| one two three  | Normal text      |
| oneTwoThree    | Camel case       |
| OneTwoThree    | Pascal case      |
| onetwothree    | All lowercase    |
| ONETWOTHREE    | All uppercase    |
| one_two_three  | Lower snake case |
| ONE_TWO_THREE  | Upper snake case |
| one-two-three  | Lower kebab case |
| ONE-TWO-THREE  | Upper kebab case |

## How It Works

The input for the search is a list of text pairs, each consisting of the text to 
find and the text to replace it with.

* **Match whole words only** – The text to find must be surrounded by 
  non-alphanumeric characters, including the start and end of the source text.
* **Ignore case** – The text to find can have any casing. It is replaced by the 
  unmodified replacement text.
* **Match case** – The text to find must have the exact same casing. It is 
  replaced by the unmodified replacement text.
* **Preserve case** – The text to find must have a casing that allows determining 
  its individual words. The words can be separated by spaces, hyphens, or 
  underscores. It is replaced by the modified replacement text to match the found 
  casing.

1. In preserve case mode, the text to find is separated into words.
2. A list of all casing variants of the text to find and the text to replace is 
   built.
3. Then the source text is parsed and the replacements are executed.

## Command-Line Tool

```
Robolina - Text find and replace tool with case preservation.

Usage: robolina [options] <path> <text-to-find> <replacement-text>

Options:
  --case-mode <mode>        Set case mode (preserve, ignore, match).
                            Default: preserve
  --match-whole-word        Only replace whole words.
  --replacements-file, -f   Optionally provide replacement options in a file.
  --recursive, -r           Process directories recursively.
  --verbose, -v             Print detailed information during processing.
  --dry-run                 Show what would be replaced without making changes.
  --no-rename               Do not rename files, only replace content.
  --extensions <list>       Semicolon-separated list of file extensions to
                            process (e.g. .cpp;.h;.txt)
  --help, -h                Display this help message.

Examples (Attention: use the --dry-run option before making file changes.):
  robolina src/ "old_name" "new_name" --case-mode preserve
  robolina src/ --replacements-file replacements.txt
  robolina src/ --replacements-file more_replacements.txt "old_name" "new_name"
  robolina --match-whole-word --recursive . "findMe" "replaceWithThis"
  robolina --extensions .cpp;.h;.txt src/ foo bar

Note: The text-to-find and the replacement-text use C-String escaping.

Replacements file syntax example:
----------------------------------------------------------------------
#This is a comment.
# valid values are preserve, ignore, match.
case-mode=preserve
# valid values are true, false.match-whole-word=false
text-to-find=foo bar
replacement-text=baz_qux
# Empty lines are ignored.

case-mode=ignore
match-whole-word=true
text-to-find=value
replacement-text=myValue
# case-mode and match-whole-word stay set for the next replacements.
----------------------------------------------------------------------
```

## Example Dry-Run Output
```
>robolina testdirectory "one two three" "hello world" --dry-run --recursive
Performing dry run.
File content would change: testdirectory\nestedtestdirectory\testfile4_one_two_three.txt
File would be renamed: testdirectory\nestedtestdirectory\testfile4_one_two_three.txt -> "testfile4_hello_world.txt"
No changes needed for file: testdirectory\nestedtestdirectory\testfile5.txt
File content would change: testdirectory\testfile1_OneTwoThree.txt
File would be renamed: testdirectory\testfile1_OneTwoThree.txt -> "testfile1_HelloWorld.txt"
No changes needed for file: testdirectory\testfile2.txt
File content would change: testdirectory\testfile3_one_two_three_text.txt
File would be renamed: testdirectory\testfile3_one_two_three_text.txt -> "testfile3_hello_world_text.txt"
Ignored because of file extension: testdirectory\testfile4_one_two_three.notouch
```

## Example Output
```
>robolina testdirectory "one two three" "hello world" --verbose
File content will change: testdirectory\testfile1_OneTwoThree.txt
File will be renamed: testdirectory\testfile1_OneTwoThree.txt -> "testfile1_HelloWorld.txt"
Updated file content.
Renamed file.
No changes needed for file: testdirectory\testfile2.txt
File content will change: testdirectory\testfile3_one_two_three_text.txt
File will be renamed: testdirectory\testfile3_one_two_three_text.txt -> "testfile3_hello_world_text.txt"
Updated file content.
Renamed file.
Ignored because of file extension: testdirectory\testfile4_one_two_three.notouch
```
