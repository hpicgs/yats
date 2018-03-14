
# 
# External dependencies
# 

find_package(Qt5OpenGL   5.8 REQUIRED)
find_package(Qt5Core     5.8 REQUIRED)
find_package(Qt5Gui      5.8 REQUIRED)


# 
# Executable name and options
# 

# Target name
set(target glraw)

# Exit here if required dependencies are not met
message(STATUS "Example ${target}")


# 
# Sources
# 

set(sources
    AssetInformation.cpp
    AssetInformation.h
)


# 
# Create library
# 

add_library(${target} STATIC ${sources})


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
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DEFAULT_INCLUDE_DIRECTORIES}
)


# 
# Libraries
# 

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    yats-include
    Qt5::Core
    Qt5::Gui
    Qt5::OpenGL
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


#
# Target Health
#

perform_health_checks(
    ${target}
    ${sources}
)
