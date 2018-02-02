
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
    ${include_path}/connection_helper.h
    ${include_path}/identifier.h
    ${include_path}/input_connector.h
    ${include_path}/lambda_task.h
    ${include_path}/output_connector.h
    ${include_path}/pipeline.h
    ${include_path}/scheduler.h
    ${include_path}/slot.h
    ${include_path}/task_configurator.h
    ${include_path}/task_container.h
    ${include_path}/task_helper.h
    ${include_path}/thread_safe_queue.h
    ${include_path}/thread_pool.h
    ${include_path}/util.h
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
