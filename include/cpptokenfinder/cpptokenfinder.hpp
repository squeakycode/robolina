// cpptokenfinder - A simple C++ token finder implementation
// Link: https://github.com/squeakycode/cpptokenfinder
// Version: 1.0.0
// Minimum required C++ Standard: C++11
// License: BSD 3-Clause License
// 
// Copyright (c) 2022, Andreas Gau
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
\brief Contains a simple C++ token finder implementation
\mainpage
C++ Token Finder {#pageTitle}
================

##Purpose##
A simple and fast implementation to efficiently find tokens in strings.


##Design Goals##
- Header-only
- KISS principle
- Easy to use and well documented
- Multi-platform
- No build warnings on common compilers
- High test coverage


##When to Use##
- You need to find multiple tokens in text strings.
- The search must be efficient.
*/
#pragma once
#include <vector>
#include <stdexcept>

namespace cpptokenfinder
{
    /**
        \brief Compares two character values for equality.
        The comparer classes are used to be able to apply different modes of comparison
        e.g., for ignoring the character casing.

        You can use a different custom comparer to find fixed size pattern tokens, e.g. token is machine???, ? is any digit.
        \code
            bool operator()(char character_of_token, char character_of_provided_text) const
            {
                if (character_of_token == '?' && character_of_provided_text >= '0' && character_of_provided_text <= '9')
                {
                    return true;
                }
                else
                {
                    return character_of_token == character_of_provided_text;
                }
            }
        \endcode

        Example for ignoring the character case:
        \code
            template <typename char_type_a, typename char_type_b>
            bool operator()(char_type_a value_lhs, char_type_b value_rhs) const
            {
                auto value_lhs_low = std::tolower(value_lhs, locale_object);
                auto value_rhs_low = std::tolower(value_rhs, locale_object);
                bool result = (value_lhs_low == value_rhs_low);
                return result;
            }
        \endcode
    */
    class token_finder_default_comparer
    {
        public:
        /**
            \brief Compares two character values for equality.
            \param[in] character_of_token         This is a character of a token to match.
            \param[in] character_of_searched_text This is a character of the searched text.
            \return Returns true if the characters match.
        */
        template <typename char_type>
        bool operator()(char_type character_of_token, char_type character_of_searched_text) const
        {
            return character_of_token == character_of_searched_text;
        }
    };

    /**
       \brief The token finder is used to efficiently find multiple tokens in text strings.
       @tparam char_type The type of the characters of the used strings, e.g. char.
       @tparam token_id_type The type of a token. This is usually an enumeration or an integral type use as index for
                             an additional data container storing more information about a token.
       @tparam invalid_token_id_type  This is usually the same type as token_id_type. In some cases it comes in handy to
                                      be able to use a different type.
                                      invalid_token_id_type must be assignable to token_id_type.
                                      token_id_type must allow equals comparison to invalid_token_id_type.
       @tparam c_invalid_token_id This is a value that is not a valid token ID.
       @tparam comparer_type This object is used for matching token and searched text characters,
                             see token_finder_default_comparer for more information.
    */
    template <typename char_type, typename token_id_type, typename invalid_token_id_type, invalid_token_id_type c_invalid_token_id, typename comparer_type = token_finder_default_comparer>
    class token_finder
    {
    protected:
        class search_tree_entry;
        typedef std::vector<search_tree_entry> search_tree_entry_list_type;

        // Used for building a search tree used for searching for tokens in texts.
        // example tokens auto, do, double and dolphin will produce a strict hierarchical tree:
        // - a->u->t->[o]
        // - d->[o]->b->l->[e]
        // -       ->l->p->h->i->[n]
        // The square brackets mark the position of a valid token indicated by the member token_id.
        // The search will match the longest possible token, e.g. for do and double.
        class search_tree_entry
        {
        public:
            search_tree_entry() = default;
            search_tree_entry(char_type c, const token_id_type& id)
                : character(c)
                , token_id(id)
            {
            }
            char_type character;
            token_id_type token_id;
            search_tree_entry_list_type next_entries;
        };

        // Used for searching texts and adding tokens when the input is a string object.
        // It is used to handle null-terminated strings and string objects alike.
        template <typename string_iterator_type>
        class string_wrapper
        {
        public:
            string_wrapper(string_iterator_type begin_string, string_iterator_type end_string)
                : pos(begin_string)
                , end(end_string)
            {
            }

            bool is_end_position() const
            {
                return pos == end;
            }

            bool is_last_character()
            {
                bool result = (pos + 1) == end;
                return result;
            }

            string_wrapper<string_iterator_type>& operator++ ()
            {
                ++pos;
                return *this;
            }

            const char_type& operator*() const
            {
                return *pos;
            }

            const string_iterator_type& get_position() const
            {
                return pos;
            }

            string_iterator_type pos;
            string_iterator_type end;
        };

        // Used for searching texts and adding tokens when the input is a null-terminated string.
        // It is used to handle null-terminated strings and string objects alike without searching
        // for the end of the null-terminated string first.
        template <typename char_iterator_type>
        class null_terminated_string_wrapper
        {
        public:
            null_terminated_string_wrapper(char_iterator_type* p_text)
                : p(p_text)
            {
            }

            bool is_end_position() const
            {
                return *p == 0;
            }

            bool is_last_character()
            {
                bool result = *(p + 1) == 0;
                return result;
            }

            null_terminated_string_wrapper& operator++ ()
            {
                ++p;
                return *this;
            }

            char_iterator_type& operator*() const
            {
                return *p;
            }

            char_iterator_type* get_position() const
            {
                return p;
            }

            char_iterator_type* p;
        };
    public:
        /**
            \brief Adds a token to be found.
            \param[in] p_token_string The token text.
            \param[in] token_id The token id.
            \pre
             - The \c token_id must not be equal c_invalid_token_id.
             - The token text must not be NULL.
             - The token text must not be an empty string.
             - The token text must not be added more than once.
            \post
             - The token is added to the internal data structure and can be found on the next call to find_token().

             Throws an std::invalid_argument exception if the preconditions are not met.
        */
        void add_token(const char_type* p_token_string, token_id_type token_id)
        {
            if (p_token_string == nullptr)
            {
                throw std::invalid_argument("Failed to add token. The token string is null.");
            }
            if (*p_token_string == 0)
            {
                throw std::invalid_argument("Failed to add token. The token string is empty.");
            }

            add_token_implementation(null_terminated_string_wrapper<const char_type>(p_token_string), token_id);
        }

        /**
            \brief Adds a token to be found.
            \tparam string_type A string object e.g. std::string.
            \param[in] token_string The token text.
            \param[in] token_id The token id.
            \pre
             - The \c token_id must not be equal c_invalid_token_id.
             - The token text must not be an empty string.
             - The token text must not be added more than once.
            \post
             - The token is added to the internal data structure and can be found on the next call to find_token().

             Throws an std::invalid_argument exception if the preconditions are not met.
        */
        template <typename string_type>
        void add_token(const string_type& token_string, token_id_type token_id)
        {
            if (token_string.empty())
            {
                throw std::invalid_argument("Failed to add token. The token string is empty.");
            }

            add_token_implementation(string_wrapper<typename string_type::const_iterator>(token_string.begin(), token_string.end()), token_id);
        }

        /**
            \brief Finds the next token in a text and returns its position and ID.
            \param[in] text The text to be searched for tokens.
            \param[out] token_begin_out Contains the token start position in \c p_text if a token has been found
                                        otherwise it is unchanged.
            \param[out] token_end_out Contains the token end position in \c p_text (one character past the last token character) if a token has been found
                                      otherwise it is unchanged.
            \param[out] token_id_out Contains the token ID if a token has been found otherwise it is unchanged.
            \return Returns true if a token has been found.
            \post
             - The longest matching token is returned if found, e.g. if tokens "do" and "double" have been added "double"
               will match in "The house has a double garage.".
        */
        bool find_token(const char_type* text, const char_type*& token_begin_out, const char_type*& token_end_out, token_id_type& token_id_out) const
        {
            bool result = false;
            if (text)
            {
                result = find_token_implementation(null_terminated_string_wrapper<const char_type>(text), token_begin_out, token_end_out, token_id_out);
            }
            return result;
        }

        /**
            \copydoc token_finder::find_token()
        */
        bool find_token(char_type* text, char_type*& token_begin_out, char_type*& token_end_out, token_id_type& token_id_out) const
        {
            bool result = false;
            if (text)
            {
                result = find_token_implementation(null_terminated_string_wrapper<char_type>(text), token_begin_out, token_end_out, token_id_out);
            }
            return result;
        }

        /**
            \copydoc token_finder::find_token()
        */
        template <typename string_type>
        bool find_token(const string_type& text, typename string_type::const_iterator& token_begin_out, typename string_type::const_iterator& token_end_out, token_id_type& token_id_out) const
        {
            bool result = find_token_implementation(string_wrapper<typename string_type::const_iterator>(text.begin(), text.end()), token_begin_out, token_end_out, token_id_out);
            return result;
        }

        /**
            \brief Finds the next token in a text and returns its position and ID.
            \param[in] text_begin The start of the text to be searched for tokens.
            \param[in] text_end The end position of the text to be searched for tokens.
            \param[out] token_begin_out Contains the token start position in \c p_text if a token has been found
                                        otherwise it is unchanged.
            \param[out] token_end_out Contains the token end position in \c p_text (one character past the last token character) if a token has been found
                                      otherwise it is unchanged.
            \param[out] token_id_out Contains the token ID if a token has been found otherwise it is unchanged.
            \return Returns true if a token has been found.
            \post
             - The longest matching token is returned if found, e.g. if tokens "do" and "double" have been added "double"
               will match in "The house has a double garage.".
        */
        template <typename iterator_type>
        bool find_token(iterator_type text_begin, iterator_type text_end, iterator_type& token_begin_out, iterator_type& token_end_out, token_id_type& token_id_out) const
        {
            bool result = find_token_implementation(string_wrapper<iterator_type>(text_begin, text_end), token_begin_out, token_end_out, token_id_out);
            return result;
        }

        /**
            \brief Clears all tokens added using add_token().
        */
        void clear()
        {
            root.clear();
        }

    protected:
        template <typename text_wrapper_type>
        void add_token_implementation(text_wrapper_type token_string, token_id_type token_id)
        {
            if (token_id == c_invalid_token_id)
            {
                throw std::invalid_argument("Failed to add token. Its token id is invalid.");
            }

            // We start with our root list of entries it contains the possible first characters of all tokens.
            search_tree_entry_list_type* p_current_search_tree_entry_list = &root;
            // Go through the string of the token and add it to the search tree
            for (text_wrapper_type character_of_token = token_string; !character_of_token.is_end_position(); ++character_of_token)
            {
                search_tree_entry_list_type* p_next_search_tree_entry_list = nullptr;
                for (search_tree_entry& entry : *p_current_search_tree_entry_list)
                {
                    // Is the character is already in our list?
                    // We do not use the comparer here for adding tokens.
                    if (entry.character == *character_of_token) // Existing search tree entry.
                    {
                        if (character_of_token.is_last_character()) // Last character of the new token?
                        {
                            if (entry.token_id == c_invalid_token_id)
                            {
                                entry.token_id = token_id;
                            }
                            else
                            {
                                // We do not allow to set a token twice even if it has the same token id.
                                throw std::invalid_argument("Failed to add token. It has already been added.");
                            }
                        }
                        p_next_search_tree_entry_list = &entry.next_entries;
                        break;
                    }
                }
                // Character not in list yet, new search tree entry.
                if (p_next_search_tree_entry_list == nullptr)
                {
                    if (character_of_token.is_last_character()) // Last character of the new token?
                    {
                        p_current_search_tree_entry_list->emplace_back(*character_of_token, token_id);
                    }
                    else // Not last character of the new token.
                    {
                        p_current_search_tree_entry_list->emplace_back(*character_of_token, c_invalid_token_id);
                    }
                    // Continue with the added entry.
                    p_next_search_tree_entry_list = &(p_current_search_tree_entry_list->back().next_entries);
                }
                // Continue with the next search tree entry.
                p_current_search_tree_entry_list = p_next_search_tree_entry_list;
            }
        }

        template <typename text_wrapper_type, typename iterator_type>
        bool find_token_implementation(text_wrapper_type text, iterator_type& token_begin_out, iterator_type& token_end_out, token_id_type& token_id_out) const
        {
            bool result = false;
            // Go through the string and search for matching tokens using the search tree.
            for (text_wrapper_type character_text = text; !character_text.is_end_position() && !result; ++character_text)
            {
                // We start with our root list of entries it contains the possible first characters of all tokens.
                const search_tree_entry_list_type* p_current_search_tree_entry_list = &root;
                // Look for a token using the search tree
                for (text_wrapper_type character_token = character_text; !character_token.is_end_position(); ++character_token)
                {
                    const search_tree_entry_list_type* p_next_search_tree_entry_list = nullptr;
                    for (const search_tree_entry& entry : *p_current_search_tree_entry_list)
                    {
                        // Is the character in our list?
                        if (comparer(entry.character, *character_token))
                        {
                            p_next_search_tree_entry_list = &entry.next_entries;
                            // Found a token?
                            if (!(entry.token_id == c_invalid_token_id))
                            {
                                result = true;
                                token_begin_out = character_text.get_position();
                                token_end_out = character_token.get_position() + 1; // The end position is one character past the last character.
                                token_id_out = entry.token_id;
                                // We keep on searching in case there is a longer token to match.
                            }
                            break;
                        }
                    }
                    if (p_next_search_tree_entry_list == nullptr) // No further character matched.
                    {
                        break;
                    }
                    else
                    {
                        p_current_search_tree_entry_list = p_next_search_tree_entry_list;
                    }
                }
            }
            return result;
        }

    protected:
        search_tree_entry_list_type root;
        comparer_type comparer;
    };
}
