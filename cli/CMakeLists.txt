# Add the executable
add_executable(robolina_cli
    main.cpp
)

# Link with the robolina library (header-only, so just need include directories)
target_include_directories(robolina_cli PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_compile_definitions(robolina_cli PRIVATE ROBOLINA_CLI_VERSION_STRING="${ROBOLINA_CLI_VERSION_STRING}")

set_target_properties(robolina_cli PROPERTIES OUTPUT_NAME robolina)

install(TARGETS robolina_cli
    RUNTIME DESTINATION bin
    RENAME robolina
)

custom_target_use_highest_warning_level(robolina_cli)
