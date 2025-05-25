add_executable(sample
    main.cpp
    )

target_include_directories(sample
PRIVATE
${PROJECT_SOURCE_DIR}/include
)
