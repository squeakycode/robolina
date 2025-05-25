add_executable(test_robolina_runner
        test_robolina.cpp
        )

target_include_directories(test_robolina_runner
PRIVATE
${PROJECT_SOURCE_DIR}/test/include
${PROJECT_SOURCE_DIR}/include
)

custom_target_use_highest_warning_level(test_robolina_runner)

add_test(
        NAME test_robolina
        COMMAND test_robolina_runner
)
