#include <robolina/robolina.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

struct CommandLineOptions
{
    std::vector<fs::path> paths;
    std::string textToFind;
    std::string replacementText;
    robolina::case_mode caseMode = robolina::case_mode::preserve_case;
    bool matchWholeWord = false;
    bool recursive = false;
    bool verbose = false;
    bool dryRun = false;
    bool allowRename = true; // New option to control file renaming
};

void printUsage()
{
    std::cout << "Robolina - Text replacement tool with case preservation\n\n"
              << "Usage: robolina [options] <path> <text-to-find> <replacement-text>\n\n"
              << "Options:\n"
              << "  --case-mode <mode>    Set case mode (preserve, ignore, match). Default: preserve\n"
              << "  --match-whole-word    Only replace whole words\n"
              << "  --recursive, -r       Process directories recursively\n"
              << "  --verbose, -v         Print detailed information during processing\n"
              << "  --dry-run             Show what would be replaced without making changes\n"
              << "  --no-rename           Do not rename files, only replace content\n"
              << "  --help, -h            Display this help message\n\n"
              << "Examples:\n"
              << "  robolina src/ \"old_name\" \"new_name\" --case-mode preserve\n"
              << "  robolina --match-whole-word --recursive . \"findMe\" \"replaceWithThis\"\n";
}

CommandLineOptions parseCommandLine(int argc, char* argv[])
{
    CommandLineOptions options;

    if (argc < 4)
    {
        printUsage();
        throw std::runtime_error("Not enough arguments");
    }

    int currentArg = 1;
    int positionalIndex = 0;
    while (currentArg < argc)
    {
        std::string arg = argv[currentArg];

        if (arg == "--help" || arg == "-h")
        {
            printUsage();
            exit(0);
        }
        else if (arg == "--match-whole-word")
        {
            options.matchWholeWord = true;
        }
        else if (arg == "--recursive" || arg == "-r")
        {
            options.recursive = true;
        }
        else if (arg == "--verbose" || arg == "-v")
        {
            options.verbose = true;
        }
        else if (arg == "--dry-run")
        {
            options.dryRun = true;
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
                options.caseMode = robolina::case_mode::preserve_case;
            }
            else if (modeValue == "ignore")
            {
                options.caseMode = robolina::case_mode::ignore_case;
            }
            else if (modeValue == "match")
            {
                options.caseMode = robolina::case_mode::match_case;
            }
            else
            {
                throw std::runtime_error("Invalid case mode: " + modeValue);
            }
        }
        else if (arg == "--no-rename")
        {
            options.allowRename = false;
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
                options.paths.push_back(arg);
            }
            else if (positionalIndex == 1)
            {
                options.textToFind = arg;
            }
            else if (positionalIndex == 2)
            {
                options.replacementText = arg;
            }
            else
            {
                throw std::runtime_error("Too many positional arguments");
            }
            positionalIndex++;
        }
        currentArg++;
    }

    if (positionalIndex < 3)
    {
        throw std::runtime_error("Missing required positional arguments");
    }

    return options;
}

bool shouldProcessFile(const fs::path& path)
{
    // Skip binary files, hidden files, etc.
    std::string file_extension = path.extension().string();

    // List of text file extensions to process
    static const char* textExtensions[] = {
        ".txt", ".md", ".c", ".cpp", ".h", ".hpp", ".cs", ".java", ".py", ".js",
        ".html", ".css", ".xml", ".json", ".yaml", ".yml", ".sh", ".bat", ".ps1",
        ".cmake", ".rst", ".tex", ".vndf", ".epdf", ".qml", ".qrc"
    };

    // Convert file_extension to lower case for case-insensitive comparison
    std::string file_extension_lower = file_extension;
    std::transform(file_extension_lower.begin(), file_extension_lower.end(), file_extension_lower.begin(), ::tolower);

    constexpr size_t numExtensions = sizeof(textExtensions) / sizeof(textExtensions[0]);
    for (size_t i = 0; i < numExtensions; ++i)
    {
        if (file_extension_lower == textExtensions[i])
        {
            return true;
        }
    }
    return false;
}

// Add function to perform find and replace on filenames
fs::path renameFileWithReplacement(const fs::path& originalPath, const CommandLineOptions& options)
{
    // Get the parent path and filename
    fs::path parentPath = originalPath.parent_path();
    std::string filename = originalPath.filename().string();
    std::string extension = originalPath.extension().string();
    std::string stemName = originalPath.stem().string();

    // Create a replacer for the filename
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement(
        options.textToFind.c_str(),
        options.replacementText.c_str(),
        options.caseMode,
        options.matchWholeWord
    );

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

void processFile(const fs::path& path, const CommandLineOptions& options)
{
    if (!fs::is_regular_file(path) || !shouldProcessFile(path))
    {
        return;
    }

    // Read file contents
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error: Could not open file " << path << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Create a vector one character larger for the null terminator
    std::vector<char> content(size + 1, '\0');
    file.read(content.data(), size);
    if (!file)
    {
        std::cerr << "Error: Failed to read file " << path << std::endl;
        return;
    }
    file.close();

    // Create replacer and add the replacement rule
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement(
        options.textToFind.c_str(),
        options.replacementText.c_str(),
        options.caseMode,
        options.matchWholeWord
    );

    // Create an output vector to hold the replaced content
    std::vector<char> newContent;

    // Define a vector sink adapter
    struct vector_sink
    {
        std::vector<char>& result;

        vector_sink(std::vector<char>& target) : result(target) {}

        void write(const char* begin, const char* end)
        {
            result.insert(result.end(), begin, end);
        }
    };

    // Perform the replacement directly using the main method with a sink
    vector_sink sink(newContent);
    replacer.find_and_replace(content.data(), size, sink);

    // Check if content was changed
    bool hasChanges = (content.size() - 1 != newContent.size()) ||
                     !std::equal(content.begin(), content.begin() + size, newContent.begin());

    // Flag to track if we need to perform a file rename
    bool needsRename = false;
    fs::path newPath = renameFileWithReplacement(path, options);
    needsRename = (newPath != path) && options.allowRename;

    if (hasChanges || needsRename)
    {
        if (options.verbose)
        {
            if (hasChanges)
            {
                std::cout << "Changes found in file content: " << path << std::endl;
            }
            if (needsRename)
            {
                std::cout << "File will be renamed: " << path << " -> " << newPath.filename() << std::endl;
            }
        }

        if (!options.dryRun)
        {
            if (hasChanges)
            {
                // If the path is different and the file already exists, we need to handle it
                if (needsRename && fs::exists(newPath))
                {
                    std::cerr << "Error: Cannot rename file, destination already exists: " << newPath << std::endl;
                    return;
                }

                // Write the content
                std::ofstream outFile(path, std::ios::binary);
                if (!outFile)
                {
                    std::cerr << "Error: Could not write to file " << path << std::endl;
                    return;
                }

                outFile.write(newContent.data(), newContent.size());
                outFile.close();

                if (needsRename)
                {
                    fs::rename(path, newPath);
                }
            }

            // If we need to rename but there were no content changes, we need to explicitly rename the file
            if (needsRename && !hasChanges)
            {
                try
                {
                    if (fs::exists(newPath))
                    {
                        std::cerr << "Error: Cannot rename file, destination already exists: " << newPath << std::endl;
                        return;
                    }
                    fs::rename(path, newPath);
                }
                catch (const fs::filesystem_error& e)
                {
                    std::cerr << "Error: Failed to rename file: " << e.what() << std::endl;
                    return;
                }
            }

            if (options.verbose)
            {
                if (hasChanges)
                {
                    std::cout << "Updated file content" << std::endl;
                }
                if (needsRename)
                {
                    std::cout << "Renamed file to: " << newPath.filename() << std::endl;
                }
            }
        }
        else if (options.verbose)
        {
            std::cout << "Dry run - no changes made" << std::endl;
        }
    }
    else if (options.verbose)
    {
        std::cout << "No changes needed for file: " << path << std::endl;
    }
}

void processPath(const fs::path& path, const CommandLineOptions& options)
{
    if (fs::is_regular_file(path))
    {
        processFile(path, options);
    }
    else if (fs::is_directory(path))
    {
        if (options.recursive)
        {
            for (const auto& entry : fs::recursive_directory_iterator(path))
            {
                if (fs::is_regular_file(entry))
                {
                    processFile(entry.path(), options);
                }
            }
        }
        else
        {
            for (const auto& entry : fs::directory_iterator(path))
            {
                if (fs::is_regular_file(entry))
                {
                    processFile(entry.path(), options);
                }
            }
        }
    }
    else
    {
        std::cerr << "Warning: Path is neither a file nor a directory: " << path << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        CommandLineOptions options = parseCommandLine(argc, argv);

        for (const auto& path : options.paths)
        {
            processPath(path, options);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

