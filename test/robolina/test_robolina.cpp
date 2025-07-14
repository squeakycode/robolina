#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <robolina/robolina.hpp>
#include <string>

// Helper function to create a replacer with a single rule
template<typename CharType>
robolina::case_preserve_replacer<CharType> create_replacer(const CharType* find, const CharType* replace,
                                                           robolina::case_mode mode, bool match_whole_word = false) {
    robolina::case_preserve_replacer<CharType> replacer;
    replacer.add_replacement(find, replace, mode, match_whole_word);
    return replacer;
}

TEST_CASE("Normal text replacement", "[robolina]")
{
    auto replacer = create_replacer<char>("one two three", "four five six", robolina::case_mode::preserve_case);

    SECTION("Basic replacement with preserve case") {
        std::string input = "This is one two three and another one two three.";
        std::string expected = "This is four five six and another four five six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("No match in text") {
        std::string input = "This has no matches.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == input);
    }

    SECTION("Empty input") {
        std::string input = "";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == input);
    }
}

TEST_CASE("Preserve case variants", "[robolina]")
{
    auto replacer = create_replacer<char>("one two three", "four five six", robolina::case_mode::preserve_case);

    SECTION("Normal text") {
        std::string input = "one two three";
        std::string expected = "four five six";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Camel case") {
        std::string input = "This is oneTwoThree.";
        std::string expected = "This is fourFiveSix.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Pascal case") {
        std::string input = "This is OneTwoThree.";
        std::string expected = "This is FourFiveSix.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("All lowercase") {
        std::string input = "This is onetwothree.";
        std::string expected = "This is fourfivesix.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("All uppercase") {
        std::string input = "This is ONETWOTHREE.";
        std::string expected = "This is FOURFIVESIX.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Lower snake case") {
        std::string input = "This is one_two_three.";
        std::string expected = "This is four_five_six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Upper snake case") {
        std::string input = "This is ONE_TWO_THREE.";
        std::string expected = "This is FOUR_FIVE_SIX.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Lower kebab case") {
        std::string input = "This is one-two-three.";
        std::string expected = "This is four-five-six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Upper kebab case") {
        std::string input = "This is ONE-TWO-THREE.";
        std::string expected = "This is FOUR-FIVE-SIX.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Match case mode", "[robolina]")
{
    auto replacer = create_replacer<char>("one two three", "four five six", robolina::case_mode::match_case);

    SECTION("Exact match") {
        std::string input = "This is one two three.";
        std::string expected = "This is four five six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Different case - no match") {
        std::string input = "This is ONE TWO THREE.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == input); // Should not match due to case difference
    }
}

TEST_CASE("Ignore case mode", "[robolina]")
{
    auto replacer = create_replacer<char>("one two three", "four five six", robolina::case_mode::ignore_case);

    SECTION("Exact match") {
        std::string input = "This is one two three.";
        std::string expected = "This is four five six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Different case - should match") {
        std::string input = "This is ONE TWO THREE.";
        std::string expected = "This is four five six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Mixed case - should match") {
        std::string input = "This is One Two Three.";
        std::string expected = "This is four five six.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Match whole word option", "[robolina]")
{
    auto replacer = create_replacer<char>("one", "four", robolina::case_mode::preserve_case, true);

    SECTION("Whole word match") {
        std::string input = "This is one word.";
        std::string expected = "This is four word.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Substring - no match with whole word enabled") {
        std::string input = "This is oneword.";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == input); // Should not match as "one" is part of "oneword"
    }
}

TEST_CASE("Multiple replacements with different modes", "[robolina]")
{
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement("one", "four", robolina::case_mode::preserve_case);
    replacer.add_replacement("two", "five", robolina::case_mode::match_case);
    replacer.add_replacement("three", "six", robolina::case_mode::ignore_case);

    SECTION("Mixed replacements") {
        std::string input = "one two THREE";
        std::string expected = "four five six";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }

    SECTION("Some matches, some non-matches") {
        std::string input = "one TWO three";
        std::string expected = "four TWO six"; // Only case-sensitive "two" shouldn't match
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Overlapping patterns", "[robolina]")
{
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement("one two", "four five", robolina::case_mode::preserve_case);
    replacer.add_replacement("two three", "five six", robolina::case_mode::preserve_case);

    SECTION("First match should be processed") {
        std::string input = "one two three";
        std::string expected = "four five three";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Edge cases", "[robolina]")
{
    SECTION("Empty text to find - should throw") {
        robolina::case_preserve_replacer<char> replacer;
        REQUIRE_THROWS_AS(
            replacer.add_replacement("", "replacement", robolina::case_mode::preserve_case),
            std::invalid_argument
        );
    }

    SECTION("Null text to find - should throw") {
        robolina::case_preserve_replacer<char> replacer;
        REQUIRE_THROWS_AS(
            replacer.add_replacement(nullptr, "replacement", robolina::case_mode::preserve_case),
            std::invalid_argument
        );
    }

    SECTION("Null replacement text - should throw") {
        robolina::case_preserve_replacer<char> replacer;
        REQUIRE_THROWS_AS(
            replacer.add_replacement("find", nullptr, robolina::case_mode::preserve_case),
            std::invalid_argument
        );
    }

    SECTION("Invalid case mode - should throw") {
        robolina::case_preserve_replacer<char> replacer;
        REQUIRE_THROWS_AS(
            replacer.add_replacement("find", "replace", static_cast<robolina::case_mode>(999)),
            std::invalid_argument
        );
    }
}

TEST_CASE("Different character types", "[robolina]")
{
    SECTION("Wide string") {
        robolina::case_preserve_replacer<wchar_t> replacer;
        replacer.add_replacement(L"one two three", L"four five six", robolina::case_mode::preserve_case);

        std::wstring input = L"This is one two three.";
        std::wstring expected = L"This is four five six.";
        std::wstring result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Overlapping finders 1", "[robolina]")
{
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement("one two", "four five", robolina::case_mode::ignore_case);
    replacer.add_replacement("two three", "five six", robolina::case_mode::preserve_case);

    SECTION("First match should be processed") {
        std::string input = "one two three";
        std::string expected = "four five three";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Name number mix 1", "[robolina]")
{
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement("one two 3 four", "five 6 seven", robolina::case_mode::preserve_case);

    SECTION("First match should be processed") {
        std::string input = "text one_two_3_four";
        std::string expected = "text five_6_seven";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Name number mix 2", "[robolina]")
{
    robolina::case_preserve_replacer<char> replacer;
    replacer.add_replacement("oneTwo3Four", "five 6 seven", robolina::case_mode::preserve_case);

    SECTION("First match should be processed") {
        std::string input = "text one_two3_four";
        std::string expected = "text five_6_seven";
        std::string result = replacer.find_and_replace(input);
        REQUIRE(result == expected);
    }
}
