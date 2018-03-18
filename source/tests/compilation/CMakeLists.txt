#
# Compilation tests
#

add_custom_target(compilation_test)

# Add a compilation test case, the target should be a cpp file with a main that is not compilable.
function(add_compilation_test target)

    # Configure a target, that builds another target in a separate process
    add_custom_target(${target}_build 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/run.cmake" ${target}_fail ${target})
    set_target_properties(${target}_build PROPERTIES FOLDER "Tests/Compilation")
    
    # Add the target that IS NOT compilable.
    add_executable(${target}_fail EXCLUDE_FROM_ALL ${target}.cpp)
    set_target_properties(${target}_fail PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "Tests/Compilation")
    target_link_libraries(${target}_fail PRIVATE ${DEFAULT_LIBRARIES} yats-include)
    target_compile_definitions(${target}_fail PRIVATE SHOULD_FAIL)
	
    # Add the same target that IS compilable.
    add_executable(${target} EXCLUDE_FROM_ALL ${target}.cpp)
    set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "Tests/Compilation")
    target_link_libraries(${target} PRIVATE ${DEFAULT_LIBRARIES} yats-include)

    # CI target
    add_dependencies(compilation_test ${target}_build)
endfunction()

set_target_properties(compilation_test PROPERTIES FOLDER "Tests/Compilation")

#
# Actual test cases
#

add_compilation_test(demo)
add_compilation_test(slot_move_constructible)
add_compilation_test(type_safe_connection)
add_compilation_test(has_options)
add_compilation_test(has_thread_constraints)
