//-----------------------------------------------------------------------------
// robolina
//-----------------------------------------------------------------------------

#include <robolina/robolina.hpp>
#include <iostream>
#include <string>

int main()
{
    robolina::case_preserve_replacer<char> replacer;

    // Add some replacements
    replacer.add_replacement("hello_world", "hiUniverse", robolina::case_mode::preserve_case);
    replacer.add_replacement("foo_bar", "baz_qux", robolina::case_mode::ignore_case);
    replacer.add_replacement("CamelCase", "snake_case", robolina::case_mode::match_case);

    // Example text to process
    std::string text = "HelloWorld! This is a CamelCase example with fOO_bar.";

    // Perform the find and replace operation
    std::string result = replacer.find_and_replace(text);

    // Output the result
    std::cout << "Original text: " << text << std::endl;
    std::cout << "Modified text: " << result << std::endl;

    // Output
    // Original text: HelloWorld! This is a CamelCase example with fOO_bar.
    // Modified text: HiUniverse! This is a snake_case example with baz_qux.
    return 0;
}
