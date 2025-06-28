cmake_minimum_required(VERSION 3.10)

# Add the executable
add_executable(robolina_cli
    main.cpp
)

# Link with the robolina library (header-only, so just need include directories)
target_include_directories(robolina_cli PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(robolina_cli PROPERTIES OUTPUT_NAME robolina)

install(TARGETS robolina_cli
    RUNTIME DESTINATION bin
    RENAME robolina
)
