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
