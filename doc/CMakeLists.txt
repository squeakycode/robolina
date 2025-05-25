cmake_minimum_required(VERSION 3.12)

set(INJECTED_PROJECT_NAME "robolina")
set(INJECTED_INPUT ${PROJECT_SOURCE_DIR}/include/robolina)
set(INJECTED_EXCLUDE_SYMBOLS "")
set(INJECTED_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include/robolina)
set(INJECTED_INCLUDE_FILE_PATTERNS "*.hpp")
set(INJECTED_STRIP_FROM_PATH ${PROJECT_SOURCE_DIR})

configure_file(${PROJECT_SOURCE_DIR}/doc/doxygen_config.in.txt ${CMAKE_CURRENT_BINARY_DIR}/doxygen_config.txt @ONLY)

add_custom_target(doxygen-doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen_config.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building doxygen documentation"
        SOURCES ${PROJECT_SOURCE_DIR}/doc/doxygen_config.in.txt
        )

source_group("" FILES ${PROJECT_SOURCE_DIR}/doc/doxygen_config.in.txt)
