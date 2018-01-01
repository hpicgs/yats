#
# Compilation tests
#

add_custom_target(compilation_test)

# Add a compilation test case, the target should be a cpp file with a main that is not compilable.
function(add_compilation_test target)

    # Configure a target, that builds another target in a separate process
    add_custom_target(${target}_build 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/run.cmake" ${target})
    set_target_properties(${target}_build PROPERTIES FOLDER "Tests/Compilation")
    
    # Add the target that is not compilable.
    add_executable(${target} EXCLUDE_FROM_ALL ${target}.cpp)
    set_target_properties(${target} PROPERTIES FOLDER "Tests/Compilation")
    
    # Add the build target to the compilation_test target that will be runned by the ci.
    add_dependencies(compilation_test ${target}_build)
endfunction()

set_target_properties(compilation_test PROPERTIES FOLDER "Tests/Compilation")

#
# Actual test cases
#

add_compilation_test(incompilable)
