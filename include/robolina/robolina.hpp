// Robolina Replace Preserve Case
// Link: https://github.com/squeakycode/robolina
// Version: 0.1.0
// Minimum required C++ Standard: C++11
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

/**
\file
\brief Contains a header only implementation of a case preserving text replacer.
\mainpage
Robolina Replace Preserve Case {#pageTitle}
==============================

##Purpose##

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

##How It Works##

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
*/
#pragma once
#include <cpptokenfinder/cpptokenfinder.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace robolina
{
    enum class case_mode
    {
        preserve_case, //!< The text to find must have a casing that allows determining its individual words. The words can be
                       //!< separated by spaces, hyphens, or underscores. It is replaced by the modified replacement text to match
                       //!< the found casing.
        ignore_case,   //!< The text to find can have any casing. It is replaced by the unmodified replacement text.
        match_case     //!< The text to find must have the exact same casing. It is replaced by the unmodified replacement text.
    };

    /**
     * \brief A template class that performs text replacements while preserving the casing style.
     *
     * The case_preserve_replacer is the main class of the Robolina library. It allows for searching and
     * replacing text patterns in various casing styles. It can detect and maintain the original casing
     * of the text being replaced (camelCase, PascalCase, snake_case, etc.), making it ideal for refactoring
     * source code files.
     *
     * Key features:
     * - Preserve case replacements: Intelligently maintains the original casing style of the replaced text
     * - Support for multiple casing styles: Handles normal text, camelCase, PascalCase, snake_case, and more
     * - Case-sensitive and case-insensitive matching options
     * - Whole word matching capability
     * - Stream-based replacement with customizable output sinks
     *
     * Example usage:
     * \code{.cpp}
     * robolina::case_preserve_replacer<char> replacer;
     * replacer.add_replacement("old_name", "new_name", robolina::case_mode::preserve_case);
     *
     * // This will replace "old_name", "oldName", "OLD_NAME", etc. with appropriate casing
     * std::string result = replacer.find_and_replace(inputText);
     * \endcode
     *
     * \tparam char_type The character type to use (char, wchar_t, etc.)
     */
    template<typename char_type>
    class case_preserve_replacer
    {
        typedef size_t token_id_type;
    public:
        /**
         * \brief Adds a replacement rule to the replacer.
         *
         * This method adds a new text replacement rule to the case_preserve_replacer.
         * Depending on the selected case mode, the replacer will handle the text accordingly.
         *
         * \param text_to_find The text to search for in the source text.
         * \param replacement_text The text to replace the found text with.
         * \param mode The case handling mode to use for this replacement.
         * \param match_whole_word If true, only match whole words bounded by non-alphanumeric characters.
         * \throws std::invalid_argument If text_to_find is null or empty, or replacement_text is null.
         */
        void add_replacement(const char_type* text_to_find, const char_type* replacement_text, case_mode mode, bool match_whole_word = false)
        {
            if (text_to_find == nullptr || *text_to_find == 0)
            {
                throw std::invalid_argument("Failed to add replacement. The text to find is null or empty.");
            }
            if (replacement_text == nullptr)
            {
                throw std::invalid_argument("Failed to add replacement. The replacement text is null.");
            }

            if (mode == case_mode::preserve_case)
            {
                std::vector<std::basic_string<char_type>> words_to_find = split_text(text_to_find);
                if (words_to_find.empty())
                {
                    throw std::invalid_argument("Failed to add replacement. The text to find does not contain any valid words.");
                }
                std::vector<std::basic_string<char_type>> words_of_replacement = split_text(replacement_text);
                // Add tokens for all casing variants
                finder.add_token(to_normal_text(words_to_find), to_normal_text(words_of_replacement), match_whole_word);
                finder.add_token(to_camel_case(words_to_find), to_camel_case(words_of_replacement), match_whole_word);
                finder.add_token(to_pascal_case(words_to_find), to_pascal_case(words_of_replacement), match_whole_word);
                finder.add_token(to_lowercase(words_to_find), to_lowercase(words_of_replacement), match_whole_word);
                finder.add_token(to_uppercase(words_to_find), to_uppercase(words_of_replacement), match_whole_word);
                finder.add_token(to_lower_snake_case(words_to_find), to_lower_snake_case(words_of_replacement), match_whole_word);
                finder.add_token(to_upper_snake_case(words_to_find), to_upper_snake_case(words_of_replacement), match_whole_word);
                finder.add_token(to_lower_kebab_case(words_to_find), to_lower_kebab_case(words_of_replacement), match_whole_word);
                finder.add_token(to_upper_kebab_case(words_to_find), to_upper_kebab_case(words_of_replacement), match_whole_word);

            }
            else if (mode == case_mode::ignore_case)
            {
                i_finder.add_token(text_to_find, replacement_text, match_whole_word);
            }
            else if (mode == case_mode::match_case)
            {
                finder.add_token(text_to_find, replacement_text, match_whole_word);
            }
            else
            {
                throw std::invalid_argument("Failed to add replacement. The case mode is invalid.");
            }
        }

        /**
         * \brief Performs find and replace operations on the given text using a sink for output.
         *
         * This method searches through the provided text for all the patterns added via add_replacement,
         * and replaces them according to their respective case modes. The results are written to the
         * provided sink, which must implement the required write methods.
         *
         * The sink must implement a method with the following signature:
         * \code{.cpp}
         * void write(const char_type* begin, const char_type* end);
         * \endcode
         *
         * Example usage with a custom sink:
         * \code{.cpp}
         * // Define a custom sink that counts characters
         * struct counting_sink {
         *     size_t char_count = 0;
         *
         *     void write(const char* begin, const char* end) {
         *         char_count += (end - begin);
         *         // You could also write to a file, stream, or other destination
         *         std::cout.write(begin, end - begin);
         *     }
         * };
         *
         * // Use the sink with find_and_replace
         * robolina::case_preserve_replacer<char> replacer;
         * replacer.add_replacement("old", "new", robolina::case_mode::preserve_case);
         *
         * const char* text = "Replace old with new, OLD with NEW, and Old with New.";
         * counting_sink sink;
         * replacer.find_and_replace(text, strlen(text), sink);
         *
         * std::cout << "\nTotal characters: " << sink.char_count << std::endl;
         * \endcode
         *
         * \param text Pointer to the text to process.
         * \param text_size Size of the text in characters.
         * \param sink A sink object that implements write methods to receive the processed text.
         */
        template<typename sink_type>
        void find_and_replace(const char_type* text, size_t text_size, sink_type& sink) const
        {
            if (text == nullptr || text_size == 0)
            {
                return;
            }

            search_context context;
            search_context i_context;
            context.full_text_begin = text;
            context.full_text_end = text + text_size;
            context.current = text;
            i_context.full_text_begin = text;
            i_context.full_text_end = text + text_size;
            i_context.current = text;

            finder.find_token(context);
            i_finder.find_token(i_context);

            for(;;)
            {
                bool context_has_token = context.token_found();
                bool i_context_has_token = i_context.token_found();

                if (!context_has_token && !i_context_has_token)
                {
                    // No more tokens found, we can stop.
                    break;
                }
                if (context_has_token && i_context_has_token)
                {
                    bool overlaps = context.overlaps(i_context);
                    // Both token finders found a token, we need to compare them.
                    if (context.token_begin < i_context.token_begin)
                    {
                        // The context of the case preserving finder is before the ignore case finder.
                        // Process the case preserving token.
                        context.write(finder, sink);
                        context.next_token();
                        finder.find_token(context);
                        i_context.advance_current_to(context.current);
                        if (overlaps)
                        {
                            i_context.next_token();
                            i_finder.find_token(i_context);
                        }
                    }
                    else
                    {
                        // The context of the ignore case finder is before or equal to the case preserving finder.
                        // Process the ignore case token.
                        i_context.write(i_finder, sink);
                        i_context.next_token();
                        i_finder.find_token(i_context);
                        context.advance_current_to(i_context.current);
                        if (overlaps)
                        {
                            context.next_token();
                            finder.find_token(context);
                        }
                    }
                }
                else if (context_has_token)
                {
                    // Only the case preserving finder found a token.
                    context.write(finder, sink);
                    context.next_token();
                    finder.find_token(context);
                }
                else if (i_context_has_token)
                {
                    // Only the ignore case finder found a token.
                    i_context.write(i_finder, sink);
                    i_context.next_token();
                    i_finder.find_token(i_context);
                }
            }
            search_context& last_used_context = context.current < i_context.current ? i_context : context;
            if( last_used_context.current < last_used_context.full_text_end)
            {
                // Write the remaining text after the last token.
                sink.write(last_used_context.current, last_used_context.full_text_end);
            }
        }

        /**
         * \brief Convenience method to perform find and replace operations on a std::basic_string.
         *
         * This method creates a string_sink adapter and delegates to the main find_and_replace method.
         *
         * \param text The text to search in.
         * \return A new string with all replacements applied.
         */
        std::basic_string<char_type> find_and_replace(const std::basic_string<char_type>& text) const
        {
            if (text.empty())
            {
                return text;
            }

            std::basic_string<char_type> result;

            // Define a string sink adapter that writes to our result string
            struct string_sink
            {
                std::basic_string<char_type>& result;

                string_sink(std::basic_string<char_type>& target) : result(target) {}

                void write(const char_type* begin, const char_type* end)
                {
                    result.append(begin, end);
                }
            };

            string_sink sink(result);
            find_and_replace(text.c_str(), text.size(), sink);

            return result;
        }

    protected:
        static const size_t c_invalid_token_id = static_cast<size_t>(-1);

        std::vector<std::basic_string<char_type>> split_text(const char_type* text) const
        {
            std::vector<std::basic_string<char_type>> words;
            if (text == nullptr || *text == 0)
            {
                return words; // Return empty vector if text is null or empty.
            }

            std::basic_string<char_type> current_word;
            for (const char_type* p = text; *p != 0; ++p)
            {
                if (*p == ' ' || *p == '-' || *p == '_') // Split by spaces, hyphens, or underscores.
                {
                    if (!current_word.empty())
                    {
                        words.push_back(current_word);
                        current_word.clear();
                    }
                }
                else
                {
                    // Check for transition from lowercase to uppercase (camelCase boundary)
                    if (!current_word.empty() &&
                        std::islower(static_cast<unsigned char>(*(p-1))) &&
                        std::isupper(static_cast<unsigned char>(*p)))
                    {
                        words.push_back(current_word);
                        current_word.clear();
                    }
                    current_word += *p;
                }
            }
            if (!current_word.empty())
            {
                words.push_back(current_word);
            }
            return words;
        }

        std::basic_string<char_type> to_normal_text(const std::vector<std::basic_string<char_type>>& words) const
        {
            std::basic_string<char_type> result;
            for (const auto& word : words)
            {
                if (!result.empty())
                {
                    result += ' '; // Add space between words.
                }
                result += word;
            }
            return result;
        }

        std::basic_string<char_type> to_camel_case(const std::vector<std::basic_string<char_type>>& words) const
        {
            std::basic_string<char_type> result;
            bool first_word = true;
            for (const auto& word : words)
            {
                if (word.empty())
                {
                    continue;
                }

                if (first_word)
                {
                    // First word starts with lowercase
                    result += std::tolower(static_cast<unsigned char>(word[0]));
                    first_word = false;
                }
                else
                {
                    // Subsequent words start with uppercase
                    result += std::toupper(static_cast<unsigned char>(word[0]));
                }

                // Rest of the word in lowercase
                for (size_t i = 1; i < word.size(); ++i)
                {
                    result += std::tolower(static_cast<unsigned char>(word[i]));
                }
            }
            return result;
        }

        std::basic_string<char_type> to_pascal_case(const std::vector<std::basic_string<char_type>>& words) const
        {
            std::basic_string<char_type> result;
            for (const auto& word : words)
            {
                if (word.empty())
                {
                    continue;
                }

                // Every word starts with uppercase
                result += std::toupper(static_cast<unsigned char>(word[0]));

                // Rest of the word in lowercase
                for (size_t i = 1; i < word.size(); ++i)
                {
                    result += std::tolower(static_cast<unsigned char>(word[i]));
                }
            }
            return result;
        }

        std::basic_string<char_type> to_lowercase(const std::vector<std::basic_string<char_type>>& words) const
        {
            std::basic_string<char_type> result;
            for (const auto& word : words)
            {
                for (const auto& c : word)
                {
                    result += std::tolower(static_cast<unsigned char>(c));
                }
            }
            return result;
        }

        std::basic_string<char_type> to_uppercase(const std::vector<std::basic_string<char_type>>& words) const
        {
            std::basic_string<char_type> result;
            for (const auto& word : words)
            {
                for (const auto& c : word)
                {
                    result += std::toupper(static_cast<unsigned char>(c));
                }
            }
            return result;
        }

        std::basic_string<char_type> to_snake_case(const std::vector<std::basic_string<char_type>>& words, bool uppercase = false) const
        {
            std::basic_string<char_type> result;
            bool first_word = true;
            for (const auto& word : words)
            {
                if (word.empty())
                {
                    continue;
                }

                if (!first_word)
                {
                    result += '_';
                }
                first_word = false;

                for (const auto& c : word)
                {
                    if (uppercase)
                    {
                        result += std::toupper(static_cast<unsigned char>(c));
                    }
                    else
                    {
                        result += std::tolower(static_cast<unsigned char>(c));
                    }
                }
            }
            return result;
        }

        std::basic_string<char_type> to_lower_snake_case(const std::vector<std::basic_string<char_type>>& words) const
        {
            return to_snake_case(words, false);
        }

        std::basic_string<char_type> to_upper_snake_case(const std::vector<std::basic_string<char_type>>& words) const
        {
            return to_snake_case(words, true);
        }

        std::basic_string<char_type> to_kebab_case(const std::vector<std::basic_string<char_type>>& words, bool uppercase = false) const
        {
            std::basic_string<char_type> result;
            bool first_word = true;
            for (const auto& word : words)
            {
                if (word.empty())
                {
                    continue;
                }

                if (!first_word)
                {
                    result += '-';
                }
                first_word = false;

                for (const auto& c : word)
                {
                    if (uppercase)
                    {
                        result += std::toupper(static_cast<unsigned char>(c));
                    }
                    else
                    {
                        result += std::tolower(static_cast<unsigned char>(c));
                    }
                }
            }
            return result;
        }

        std::basic_string<char_type> to_lower_kebab_case(const std::vector<std::basic_string<char_type>>& words) const
        {
            return to_kebab_case(words, false);
        }

        std::basic_string<char_type> to_upper_kebab_case(const std::vector<std::basic_string<char_type>>& words) const
        {
            return to_kebab_case(words, true);
        }

        struct replacement_entry
        {
            replacement_entry() = default;
            replacement_entry(std::basic_string<char_type>&& text, bool match_whole_word)
                : replacement_text(std::move(text))
                , match_whole_word(match_whole_word)
            {
            }

            std::basic_string<char_type> replacement_text;
            bool match_whole_word = false; //!< If true, the text to find must be a whole word.
        };

        class token_finder_ignore_case_comparer
        {
        public:
            template <typename char_type_a, typename char_type_b>
            bool operator()(char_type_a value_lhs, char_type_b value_rhs) const
            {
                auto value_lhs_low = std::tolower(value_lhs);
                auto value_rhs_low = std::tolower(value_rhs);
                bool result = (value_lhs_low == value_rhs_low);
                return result;
            }
        };

        struct search_context
        {
            const char_type* full_text_begin = nullptr; //!< The text to search in.
            const char_type* full_text_end = nullptr; //!< The text to search in.
            const char_type* current = nullptr; //!< The text to search in. Current position in the text.
            const char_type* token_begin = nullptr; //!< The begin of a token or nullptr if no token is found.
            const char_type* token_end = nullptr; //!< The end of a token or nullptr if no token is found.
            size_t token_id = c_invalid_token_id; //!< The ID of the found token or c_invalid_token_id if no token is found.

            template<typename finder_type, typename sink_type>
            void write(const finder_type& finder, sink_type& sink) const
            {
                // write the text before the token
                sink.write(current, token_begin);
                // write the replacement text
                if (token_id != c_invalid_token_id)
                {
                    // If we have a valid token ID, we can use it to get the replacement text.
                    const auto& replacement = finder.replacement_entries[token_id];
                    sink.write(replacement.replacement_text.data(), replacement.replacement_text.data() + replacement.replacement_text.size());
                }
            }

            void advance_current_to(const char_type* new_current)
            {
                current = new_current;
                if (token_begin && token_begin < new_current) // new_current is after the token begin -> drop the token
                {
                    token_begin = nullptr;
                    token_end = nullptr;
                    token_id = c_invalid_token_id;
                }
            }

            void next_token()
            {
                if (token_end != nullptr)
                {
                    current = token_end; // Move current position to the end of the found token.
                }
            }

            bool overlaps(const search_context& other) const
            {
                return (token_begin < other.token_end && other.token_begin < token_end) ||
                       (token_begin == other.token_begin); // Check for overlapping or same starting position

                // Examples with positions 0-9:
                // Case 1: Regular overlap
                // Token A: [2,5) - token_begin=2, token_end=5
                // Token B: [4,7) - token_begin=4, token_end=7
                // Result: true (A.begin=2 < B.end=7 && B.begin=4 < A.end=5)
                //
                // Case 2: One token completely inside another
                // Token A: [1,8) - token_begin=1, token_end=8
                // Token B: [3,6) - token_begin=3, token_end=6
                // Result: true (A.begin=1 < B.end=6 && B.begin=3 < A.end=8)
                //
                // Case 3: Same starting position
                // Token A: [2,5) - token_begin=2, token_end=5
                // Token B: [2,7) - token_begin=2, token_end=7
                // Result: true (A.begin==B.begin)
                //
                // Case 4: Same ending position
                // Token A: [2,6) - token_begin=2, token_end=6
                // Token B: [4,6) - token_begin=4, token_end=6
                // Result: true (A.begin=2 < B.end=6 && B.begin=4 < A.end=6)
                //
                // Case 5: Adjacent tokens (not overlapping)
                // Token A: [1,3) - token_begin=1, token_end=3
                // Token B: [3,5) - token_begin=3, token_end=5
                // Result: false (!(A.begin=1 < B.end=5 && B.begin=3 < A.end=3))
                //
                // Case 6: No overlap
                // Token A: [1,3) - token_begin=1, token_end=3
                // Token B: [5,8) - token_begin=5, token_end=8
                // Result: false (!(A.begin=1 < B.end=8 && B.begin=5 < A.end=3))
            }

            bool token_found() const
            {
                return token_id != c_invalid_token_id;
            }
        };

        template<typename comparer_type>
        struct token_finder_data
        {
            typedef cpptokenfinder::token_finder<char_type, token_id_type, token_id_type, c_invalid_token_id, comparer_type> token_finder_t;
            token_finder_t token_finder;
            std::vector<replacement_entry> replacement_entries;

            bool find_token(search_context& context) const
            {
                bool result = true;
                const char_type* preserve_current = context.current;
                while(result)
                {
                    result = token_finder.find_token(context.current, context.token_begin, context.token_end, context.token_id);
                    if (result)
                    {
                        // Check if the token matches the whole word condition.
                        const auto& replacement = replacement_entries[context.token_id];
                        if (replacement.match_whole_word)
                        {
                            // Check if the token is a whole word.
                            bool is_whole_word = true;
                            if (context.token_begin > context.full_text_begin &&
                                std::isalnum(static_cast<unsigned char>(*(context.token_begin - 1))))
                            {
                                is_whole_word = false; // Not a whole word, previous character is alphanumeric.
                            }
                            if (is_whole_word && context.token_end < context.full_text_end &&
                                std::isalnum(static_cast<unsigned char>(*context.token_end)))
                            {
                                is_whole_word = false; // Not a whole word, next character is alphanumeric.
                            }
                            if (is_whole_word)
                            {
                                break; // Found a valid token that matches the whole word condition.
                            }
                            else
                            {
                                context.next_token(); // Move to the next position in the text.
                            }
                        }
                        else
                        {
                            break; // Found a valid token that does not require whole word matching.
                        }
                    }
                }
                context.current = preserve_current; // Restore the current position in the text.
                if (!result)
                {
                    context.token_begin = nullptr; // No token found.
                    context.token_end = nullptr; // No token found.
                    context.token_id = c_invalid_token_id; // No token found.
                }
                return false; // No token found.
            }

            bool add_token(std::basic_string<char_type> text_to_find, std::basic_string<char_type> replacement_text, bool match_whole_word)
            {
                // check if we already have a token for the text to find
                auto token_begin = text_to_find.cbegin();
                auto token_end = text_to_find.cend();
                token_id_type token_id = c_invalid_token_id;
                if ( token_finder.find_token(text_to_find, token_begin, token_end, token_id)
                    && token_begin == text_to_find.begin()
                    && token_end == text_to_find.end()
                )
                {
                    return false;
                }
                token_finder.add_token(text_to_find, replacement_entries.size() /* token_id */);
                replacement_entries.emplace_back(std::move(replacement_text), match_whole_word);
                return true;
            }
        };

        token_finder_data<cpptokenfinder::token_finder_default_comparer> finder;
        token_finder_data<token_finder_ignore_case_comparer> i_finder;
    };
}
