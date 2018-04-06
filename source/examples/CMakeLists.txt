
# Check if examples are enabled
if(NOT OPTION_BUILD_EXAMPLES)
    return()
endif()

# Example applications
add_subdirectory(citric_acid_cycle)
add_subdirectory(pipeline)
