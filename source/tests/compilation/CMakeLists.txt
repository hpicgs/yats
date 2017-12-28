#
# Compilation tests
#

add_custom_target(compilation_test)

function(add_compilation_test target)
    add_custom_target(${target}_build 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/run.cmake" ${target})
    add_executable(${target} EXCLUDE_FROM_ALL ${target}.cpp)
    add_dependencies(compilation_test ${target}_build)
endfunction()

#
# Actual test cases
#

add_compilation_test(incompilable)
