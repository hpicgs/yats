
# 
# External dependencies
# 

find_package(${META_PROJECT_NAME} REQUIRED HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../")

# 
# Executable name and options
# 

# Target name
set(target yats-test)
message(STATUS "Test ${target}")


# 
# Sources
# 

set(sources
    connection_test.cpp
    custom_constructor_test.cpp
    identifier_test.cpp
    main.cpp
    pipeline_test.cpp
    queue_test.cpp
    scheduler_adv_test.cpp
    scheduler_test.cpp
    slot_test.cpp
    task_configurator_test.cpp
    task_container_test.cpp
    util_test.cpp
    yats_test.cpp
)

set(includes
    test_util.h
)

# 
# Create executable
# 

# Build executable
add_executable(${target}
    ${sources}
    ${includes}
)


# 
# Project options
# 

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)


# 
# Include directories
# 

target_include_directories(${target}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${DEFAULT_INCLUDE_DIRECTORIES}
    ${PROJECT_BINARY_DIR}/source/include
)


# 
# Libraries
# 

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    yats-include
    gmock-dev
)

# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}
)


# 
# Compile options
# 

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)


# 
# Linker options
# 

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
)
