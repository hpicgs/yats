# Try to compile an executable that was configured by the calling process
execute_process(COMMAND "${CMAKE_COMMAND}" --build . --target ${CMAKE_ARGV3} RESULT_VARIABLE res_var OUTPUT_QUIET)

# We fail here if the compilation WAS successful.
if("${res_var}" STREQUAL "0")
  message(FATAL_ERROR "Compilation did not fail!")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" --build . --target ${CMAKE_ARGV4} RESULT_VARIABLE res_var OUTPUT_QUIET)

# We fail here if the compilation WAS NOT successful.
if(NOT("${res_var}" STREQUAL "0"))
  message(FATAL_ERROR "Compilation failed!")
endif()