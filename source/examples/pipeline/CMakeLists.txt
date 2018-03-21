
# 
# External dependencies
# 

find_package(Qt5OpenGL   5.8 REQUIRED)
find_package(Qt5Core     5.8 REQUIRED)
find_package(Qt5Gui      5.8 REQUIRED)
find_package(glbinding REQUIRED)
find_package(globjects REQUIRED)
find_package(glm REQUIRED)


# 
# Executable name and options
# 

# Target name
set(target pipeline)

# Exit here if required dependencies are not met
message(STATUS "Example ${target}")


# 
# Sources
# 

set(sources
    gl.cpp
    main.cpp
    window.cpp
    window.h
)


# 
# Create executable
# 

# Build executable
add_executable(${target}
    MACOSX_BUNDLE
    ${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})


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
    glbinding::glbinding
    globjects::globjects
    glm
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


# 
# Deployment
# 

# Executable
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT examples
    BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT examples
)
