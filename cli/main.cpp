// Robolina Replace Preserve Case
// Link: https://github.com/squeakycode/robolina
// Uses: https://github.com/squeakycode/cpptokenfinder
//
// Minimum required C++ Standard: C++17
// License: BSD 3-Clause License
//
// Copyright (c) 2025, Andreas Gau
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <robolina/robolina.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#if defined(_MSC_VER) || defined(_WIN32)  || defined(_WIN64)
#define ROBOLINA_WINDOWS
#endif

namespace fs = std::filesystem;

#if defined(ROBOLINA_WINDOWS)
#include <windows.h>
std::wstring convertToWideString(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wstr(size_needed, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size_needed);
    return wstr;
}
inline std::string toString(const fs::path& path)
{
    // for Windows, convert the path to a UTF-8 string
    return { reinterpret_cast<const char*>(path.u8string().c_str()) };
}
#else
inline std::string toString(const fs::path& path)
{
    // use the native string representation of the path
    return path.string();
}
#endif

struct ReplacementOptions
{
    std::string textToFind;
    std::string replacementText;
    robolina::case_mode caseMode = robolina::case_mode::preserve_case;
    bool matchWholeWord = false;
};

struct ProcessingOptions
{
    bool recursive = false;
    bool verbose = false;
    bool dryRun = false;
    bool allowRename = true; // New option to control file renaming
    std::vector<std::string> customExtensions; // New: custom file extensions
};

struct CommandLineOptions
{
    fs::path filenameOrPath;
    ProcessingOptions processingOptions;
    std::vector<ReplacementOptions> replacements;
};

void printUsage()
{
    std::cout << "Robolina - v" << ROBOLINA_CLI_VERSION_STRING << " - Text find and replace tool with case preservation." << std::endl << std::endl
              << "Usage: robolina [options] <path> <text-to-find> <replacement-text>" << std::endl << std::endl
              << "Options:" << std::endl
              << "  --case-mode <mode>        Set case mode (preserve, ignore, match)." << std::endl
              << "                            Default: preserve" << std::endl
              << "  --match-whole-word        Only replace whole words." << std::endl
              << "  --replacements-file, -f   Optionally provide replacement options in a file." << std::endl
              << "  --recursive, -r           Process directories recursively." << std::endl
              << "  --verbose, -v             Print detailed information during processing." << std::endl
              << "  --dry-run                 Show what would be replaced without making changes." << std::endl
              << "  --no-rename               Do not rename files, only replace content." << std::endl
              << "  --extensions <list>       Semicolon-separated list of file extensions to" << std::endl
              << "                            process (e.g. .cpp;.h;.txt)" << std::endl
              << "  --help, -h                Display this help message." << std::endl
              << std::endl
              << "Examples (Attention: use the --dry-run option before making file changes.):" << std::endl
              << R"(  robolina src/ "old_name" "new_name" --case-mode preserve)" << std::endl
              << R"(  robolina src/ --replacements-file replacements.txt)" << std::endl
              << R"(  robolina src/ --replacements-file more_replacements.txt "old_name" "new_name")" << std::endl
              << R"(  robolina --match-whole-word --recursive . "findMe" "replaceWithThis")" << std::endl
              << R"(  robolina --extensions .cpp;.h;.txt src/ foo bar)" << std::endl
              << std::endl
              << "Note: The text-to-find and the replacement-text use C-String escaping." << std::endl << std::endl
              << "Replacements file syntax example:" << std::endl
              << "----------------------------------------------------------------------" << std::endl
              << "#This is a comment." << std::endl
              << "# valid values are preserve, ignore, match." << std::endl
              << "case-mode=preserve" << std::endl
              << "# valid values are true, false."
              << "match-whole-word=false" << std::endl
              << "text-to-find=foo bar" << std::endl
              << "replacement-text=baz_qux" << std::endl
              << "# shorter syntax using text-to-find-->replacement-text pairs" << std::endl
              << "pair=value3-->myValue3" << std::endl
              << "value4-->myValue4" << std::endl
              << "# Empty lines are ignored." << std::endl
              << std::endl
              << "case-mode=ignore" << std::endl
              << "match-whole-word=true" << std::endl
              << "text-to-find=value" << std::endl
              << "replacement-text=myValue" << std::endl
              << "# case-mode and match-whole-word stay set for the next replacements." << std::endl
              << "----------------------------------------------------------------------" << std::endl;
}

std::string convertCStringSyntax(const std::string& input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\')
        {
            if (i + 1 < input.size())
            {
                switch (input[i + 1])
                {
                    case 'r':
                        result += '\r';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '"':
                        result += '"';
                        break;
                    case '\'':
                        result += '\'';
                        break;
                    default:
                        result += input[i + 1];
                        break;
                }
                ++i; // Skip the next character
            }
        }
        else
        {
            result += input[i];
        }
    }

    return result;
}

void loadOptionsFromFile(const std::string& filePath, std::vector<ReplacementOptions>& replacementOptions)
{
#if defined(ROBOLINA_WINDOWS)
    std::ifstream file(convertToWideString(filePath));
#else
    std::ifstream file(filePath);
#endif
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open options file: " + filePath);
    }

    ReplacementOptions currentOptions;
    bool textToFindSet = false;
    bool replacementTextSet = false;
    std::string line;
    size_t lineCount = 0;
    while (std::getline(file, line))
    {
        ++lineCount;
        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream lineStream(line);
        std::string key, value;
        if (std::getline(lineStream, key, '=') && std::getline(lineStream, value))
        {
            if (key == "text-to-find")
            {
                currentOptions.textToFind = value;
                textToFindSet = true;
            }
            else if (key == "replacement-text")
            {
                currentOptions.replacementText = value;
                replacementTextSet = true;
            }
            else if (key == "match-whole-word")
            {
                currentOptions.matchWholeWord = (value == "true");
                if (!currentOptions.matchWholeWord && value != "false")
                {
                    throw std::runtime_error("Invalid match-whole-word ('true' or 'false' expected) in file, line " + std::to_string(lineCount) + ": " + value);
                }
            }
            else if (key == "case-mode")
            {
                if (value == "preserve")
                {
                    currentOptions.caseMode = robolina::case_mode::preserve_case;
                }
                else if (value == "ignore")
                {
                    currentOptions.caseMode = robolina::case_mode::ignore_case;
                }
                else if (value == "match")
                {
                    currentOptions.caseMode = robolina::case_mode::match_case;
                }
                else
                {
                    throw std::runtime_error("Invalid case mode in file, line " + std::to_string(lineCount) + ": " + value);
                }
            }
            else if (key == "pair")
            {
                size_t delimiterPos = value.find("-->");
                if (delimiterPos == std::string::npos)
                {
                    throw std::runtime_error("Invalid replace syntax in file, line " + std::to_string(lineCount) + ": " + value);
                }

                currentOptions.textToFind = value.substr(0, delimiterPos);
                currentOptions.replacementText = value.substr(delimiterPos + 3); // Skip the --> delimiter
                textToFindSet = true;
                replacementTextSet = true;
            }
            else
            {
                throw std::runtime_error("Unknown name in file, line " + std::to_string(lineCount) + ": " + key);
            }
        }
        else
        {
            size_t delimiterPos = line.find("-->");
            if (delimiterPos == std::string::npos)
            {
                throw std::runtime_error("Bad syntax in file, line " + std::to_string(lineCount));
            }

            currentOptions.textToFind = line.substr(0, delimiterPos);
            currentOptions.replacementText = line.substr(delimiterPos + 3); // Skip the --> delimiter
            textToFindSet = true;
            replacementTextSet = true;
        }

        // Add currentOptions to the list if all required fields are set
        if (textToFindSet && replacementTextSet)
        {
            replacementOptions.push_back(currentOptions);
            replacementTextSet = false;
            textToFindSet = false;
        }
    }
}

CommandLineOptions parseCommandLine(int argc, char* argv[])
{
    CommandLineOptions options;
    ReplacementOptions cliReplacementOptions;

    // Check for help option before any error or argument count checks
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            exit(0);
        }
    }

    int currentArg = 1;
    int positionalIndex = 0;
    bool replacementsFileUsed = false;
    while (currentArg < argc)
    {
        std::string arg = argv[currentArg];

        if (arg == "--match-whole-word")
        {
            cliReplacementOptions.matchWholeWord = true;
        }
        else if (arg == "--recursive" || arg == "-r")
        {
            options.processingOptions.recursive = true;
        }
        else if (arg == "--verbose" || arg == "-v")
        {
            options.processingOptions.verbose = true;
        }
        else if (arg == "--dry-run")
        {
            options.processingOptions.dryRun = true;
            options.processingOptions.verbose = true;
        }
        else if (arg == "--case-mode")
        {
            if (currentArg + 1 >= argc)
            {
                throw std::runtime_error("Missing value for --case-mode");
            }
            std::string modeValue = argv[++currentArg];
            if (modeValue == "preserve")
            {
                cliReplacementOptions.caseMode = robolina::case_mode::preserve_case;
            }
            else if (modeValue == "ignore")
            {
                cliReplacementOptions.caseMode = robolina::case_mode::ignore_case;
            }
            else if (modeValue == "match")
            {
                cliReplacementOptions.caseMode = robolina::case_mode::match_case;
            }
            else
            {
                throw std::runtime_error("Invalid case mode: " + modeValue);
            }
        }
        else if (arg == "--no-rename")
        {
            options.processingOptions.allowRename = false;
        }
        else if (arg == "--extensions")
        {
            if (currentArg + 1 >= argc)
            {
                throw std::runtime_error("Missing value for --extensions");
            }
            std::string extList = argv[++currentArg];
            size_t start = 0;
            size_t end = 0;
            while ((end = extList.find(';', start)) != std::string::npos)
            {
                std::string ext = extList.substr(start, end - start);
                if (!ext.empty()) options.processingOptions.customExtensions.push_back(ext);
                start = end + 1;
            }
            std::string lastExt = extList.substr(start);
            if (!lastExt.empty()) options.processingOptions.customExtensions.push_back(lastExt);
            if (options.processingOptions.customExtensions.empty())
            {
                throw std::runtime_error("No valid extensions provided in --extensions");
            }
        }
        else if (arg == "--replacements-file" || arg == "-f") {
            if (currentArg + 1 >= argc)
            {
                throw std::runtime_error("Missing value for --replacements-file");
            }
            std::string filePath = argv[++currentArg];
            loadOptionsFromFile(filePath, options.replacements);
            replacementsFileUsed = true;
        }
        else if (arg[0] == '-')
        {
            throw std::runtime_error("Unknown option: " + arg);
        }
        else
        {
            // Use positionalIndex to determine which positional argument this is
            if (positionalIndex == 0)
            {
#if defined(ROBOLINA_WINDOWS)
                options.filenameOrPath = convertToWideString(arg);
#else
                options.filenameOrPath = arg;
#endif
            }
            else if (positionalIndex == 1)
            {
                cliReplacementOptions.textToFind = arg;
            }
            else if (positionalIndex == 2)
            {
                cliReplacementOptions.replacementText = arg;
            }
            else
            {
                throw std::runtime_error("Too many positional arguments");
            }
            positionalIndex++;
        }
        currentArg++;
    }
    if (positionalIndex == 1 && replacementsFileUsed)
    {
        //using replacements file
    }
    else if (positionalIndex == 3)
    {
        options.replacements.push_back(cliReplacementOptions);
    }
    else
    {
        throw std::runtime_error("Missing required positional arguments");
    }

    return options;
}

bool shouldProcessFile(const fs::path& path, const std::vector<std::string>& customExtensions)
{
    // Skip binary files, hidden files, etc.
    std::string file_extension = toString(path.extension());

    // Convert file_extension to lower case for case-insensitive comparison
    std::string file_extension_lower = file_extension;
    std::transform(file_extension_lower.begin(), file_extension_lower.end(), file_extension_lower.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });

    if (!customExtensions.empty())
    {
        for (const auto& ext : customExtensions)
        {
            std::string ext_lower = ext;
            std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
            if (file_extension_lower == ext_lower)
            {
                return true;
            }
        }
        return false;
    }

    // List of text file extensions to process
    static const char* textExtensions[] = {
        ".txt", ".md", ".c", ".cpp", ".h", ".hpp", ".cs", ".java", ".py", ".js",
        ".html", ".css", ".xml", ".json", ".yaml", ".yml", ".sh", ".bat", ".ps1",
        ".cmake", ".rst", ".tex", ".vndf", ".epdf", ".qml", ".qrc"
    };

    for (const char* const& ext : textExtensions)
    {
        if (file_extension_lower == ext)
        {
            return true;
        }
    }
    return false;
}

fs::path renameFileWithReplacement(const fs::path& originalPath, const robolina::case_preserve_replacer<char>& replacer)
{
    // Get the parent path and filename
    fs::path parentPath = originalPath.parent_path();
    std::string filename = toString(originalPath.filename());
    std::string extension = toString(originalPath.extension());
    std::string stemName = toString(originalPath.stem());

    // Replace in the stem name
    std::string newStemName = replacer.find_and_replace(stemName);

    // If the stem name didn't change, return the original path
    if (stemName == newStemName)
    {
        return originalPath;
    }

    // Create the new filename with the original extension
    std::string newFilename = newStemName + extension;
    fs::path newPath = parentPath / newFilename;

    return newPath;
}

void processFile(const fs::path& path, const robolina::case_preserve_replacer<char>& replacer, const ProcessingOptions& options)
{
    if (!fs::is_regular_file(path))
    {
        return;
    }
    if (!shouldProcessFile(path, options.customExtensions))
    {
        if (options.verbose)
        {
            std::cout << "Ignored because of file extension: " << toString(path) << std::endl;
        }
        return;
    }

    // Read file contents
    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        if (options.dryRun)
        {
            std::cerr << "Error: Could not open file " << toString(path) << std::endl;
            return;
        }
        else
        {
            throw std::runtime_error("Could not open file " + toString(path));
        }
    }

    file.seekg(0, std::ios::end);
    const auto fileSizeInByte = file.tellg();
    file.seekg(0, std::ios::beg);

    // Create a vector one character larger for the null terminator
    std::vector<char> content(static_cast<size_t>(fileSizeInByte) + 1, '\0');
    file.read(content.data(), fileSizeInByte);
    if (!file)
    {
        if (options.dryRun)
        {
            std::cerr << "Error: Failed to read file " << toString(path) << std::endl;
            return;
        }
        else
        {
            throw std::runtime_error("Failed to read file " + toString(path));
        }
    }
    file.close();

    // Create an output vector to hold the replaced content
    std::vector<char> newContent;
    newContent.reserve(fileSizeInByte);

    // Define a vector sink adapter
    struct vector_sink
    {
        std::vector<char>& result;

        explicit  vector_sink(std::vector<char>& target) : result(target) {}

        void write(const char* begin, const char* end) const
        {
            result.insert(result.end(), begin, end);
        }
    };

    // Perform the replacement directly using the main method with a sink
    vector_sink sink(newContent);
    replacer.find_and_replace(content.data(), fileSizeInByte, sink);

    // Check if content was changed
    bool hasChanges = (content.size() - 1 != newContent.size()) ||
                     !std::equal(content.begin(), content.begin() + fileSizeInByte, newContent.begin());

    // Flag to track if we need to perform a file rename
    fs::path newPath = renameFileWithReplacement(path, replacer);
    bool needsRename = (newPath != path) && options.allowRename;

    if (hasChanges || needsRename)
    {
        if (options.verbose)
        {
            if (hasChanges)
            {
                if (options.dryRun)
                {
                    std::cout << "File content would change: " << toString(path) << std::endl;
                }
                else
                {
                    std::cout << "File content will change: " << toString(path) << std::endl;
                }
            }
            if (needsRename)
            {
                if (options.dryRun)
                {
                    std::cout << "File would be renamed: " << toString(path) << " -> " << toString(newPath.filename()) << std::endl;
                }
                else
                {
                    std::cout << "File will be renamed: " << toString(path) << " -> " << toString(newPath.filename()) << std::endl;
                }
                if (fs::exists(newPath))
                {
                    std::cerr << "Error: Cannot rename file, destination already exists: " << toString(newPath) << std::endl;
                }
            }
        }

        if (!options.dryRun)
        {
            if (hasChanges)
            {
                // If the path is different and the file already exists, we need to handle it
                if (needsRename && fs::exists(newPath))
                {
                    throw std::runtime_error("Cannot rename file, destination already exists: " + toString(newPath));
                }

                // Write the content
                std::ofstream outFile(path, std::ios::binary);
                if (!outFile)
                {
                    throw std::runtime_error("Could not write to file " + toString(path));
                }

                outFile.write(newContent.data(), static_cast<std::streamsize>(newContent.size()));
                outFile.close();
                if (options.verbose)
                {
                    std::cout << "Updated file content." << std::endl;
                }

                if (needsRename)
                {
                    fs::rename(path, newPath);
                    if (options.verbose)
                    {
                        std::cout << "Renamed file." << std::endl;
                    }
                }
            }

            // If we need to rename but there were no content changes, we need to explicitly rename the file
            if (needsRename && !hasChanges)
            {
                if (fs::exists(newPath))
                {
                    throw std::runtime_error("Cannot rename file, destination already exists: " + toString(newPath));
                }
                fs::rename(path, newPath);
                if (options.verbose)
                {
                    std::cout << "Renamed file." << std::endl;
                }
            }
        }
    }
    else if (options.verbose)
    {
        std::cout << "No changes needed for file: " << toString(path) << std::endl;
    }
}

void processPath(const fs::path& path, const CommandLineOptions& options)
{
    // Create replacer and add the replacement rules
    robolina::case_preserve_replacer<char> replacer;
    for (const auto& replacement : options.replacements )
    {
        replacer.add_replacement(
            convertCStringSyntax(replacement.textToFind).c_str(),
            convertCStringSyntax(replacement.replacementText).c_str(),
            replacement.caseMode,
            replacement.matchWholeWord
        );
    }

    if (fs::is_regular_file(path))
    {
        processFile(path, replacer, options.processingOptions);
    }
    else if (fs::is_directory(path))
    {
        if (options.processingOptions.recursive)
        {
            for (const auto& entry : fs::recursive_directory_iterator(path))
            {
                if (fs::is_regular_file(entry))
                {
                    processFile(entry.path(), replacer, options.processingOptions);
                }
            }
        }
        else
        {
            for (const auto& entry : fs::directory_iterator(path))
            {
                if (fs::is_regular_file(entry))
                {
                    processFile(entry.path(), replacer, options.processingOptions);
                }
            }
        }
    }
    else
    {
        throw std::runtime_error("Path is neither a file nor a directory: " + toString(path));
    }
}

int main(int argc, char* argv[])
{
#if defined(ROBOLINA_WINDOWS)
    // Set the encoding of the output to UTF-8 on Windows
    SetConsoleOutputCP(CP_UTF8);

    std::vector<std::vector<char>> utf8Buffers;
    std::vector<char*> utf8Argv;
    {
        // Convert command line arguments to UTF-8 on Windows
        int wargc;
        wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if (wargv)
        {
            for (int i = 0; i < wargc; ++i)
            {
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
                std::vector<char> buffer(size_needed);
                WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, buffer.data(), size_needed, NULL, NULL);
                utf8Buffers.push_back(std::move(buffer));
            }
            LocalFree(wargv);
            for (auto& buf : utf8Buffers)
            {
                utf8Argv.push_back(buf.data());
            }
            argv = utf8Argv.data();
            argc = static_cast<int>(utf8Argv.size());
        }
    }
#endif

    try
    {
        CommandLineOptions options = parseCommandLine(argc, argv);
        if (options.processingOptions.dryRun && options.processingOptions.verbose)
        {
            std::cout << "Performing dry run." << std::endl;
        }
        processPath(options.filenameOrPath, options);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Error: Unexpected exception caught." << std::endl;
        return 1;
    }
}
