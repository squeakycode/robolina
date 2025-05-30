cmake_minimum_required(VERSION 3.12)

# We are building C++ Token Finder
project(robolina CXX)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    cmake_policy(SET CMP0074 NEW) # The NEW behavior for this policy is to use <PackageName>_ROOT variables.
endif()

set (CMAKE_CXX_STANDARD 17) # Updated to C++17 for std::filesystem

# Use highest warning levels
macro(custom_target_use_highest_warning_level TargetName)
if(MSVC)
  target_compile_options(${TargetName} PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(${TargetName} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
endmacro(custom_target_use_highest_warning_level)

option(ROBOLINA_BUILD_TESTS "Determines whether to build tests." ON)
if(ROBOLINA_BUILD_TESTS)
    message("Building tests.")
    include(CTest)
    add_subdirectory(test/robolina)
endif()

option(ROBOLINA_GENERATE_DOXYGEN_DOCUMENTATION "API documentation will be generated using Doxygen if on." ON)
if(ROBOLINA_GENERATE_DOXYGEN_DOCUMENTATION)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        message("Generating Doxygen documentation.")
        add_subdirectory(doc)
    else()
        message("Doxygen is not installed. Skipping Doxygen documentation generation")
    endif()
endif()

add_subdirectory(sample)
add_subdirectory(cli)

# Packaging support
include(CPack)

set(CPACK_PACKAGE_NAME "robolina")
set(CPACK_PACKAGE_VENDOR "YourCompany")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Robolina - Text replacement tool with case preservation")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_CONTACT "andreas.gau@gmx.org")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Set package type defaults
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ")
elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

# Optionally, install the CLI binary
install(TARGETS robolina_cli DESTINATION bin)

include(CPack)

