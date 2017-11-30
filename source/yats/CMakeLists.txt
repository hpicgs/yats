
# 
# External dependencies
# 

# find_package(THIRDPARTY REQUIRED)


# 
# Library name and options
# 

# Target name
set(target yats)

# Exit here if required dependencies are not met
message(STATUS "Header Only Library ${target}")

# 
# Sources
# 

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")

set(headers
    ${include_path}/Input.h
    ${include_path}/InputProxy.h
    ${include_path}/Nodecontainer.h
    ${include_path}/Nodeconfigurator.h
    ${include_path}/OutputProxy.h
    ${include_path}/Pipeline.h
	${include_path}/Util.h
)

# Group source files
set(header_group "Header Files (API)")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$" 
    ${header_group} ${headers})


# 
# Create library
# 

# Visual Target
add_custom_target(${target} DEPENDS ${headers} SOURCES ${headers})

# Hidden target used for other libraries
add_library(${target}-include INTERFACE)
target_include_directories(${target}-include INTERFACE include)

#
# Target Health
#

perform_health_checks(
    ${target}
    ${headers}
)


# 
# Deployment
# 

# Header files
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)
