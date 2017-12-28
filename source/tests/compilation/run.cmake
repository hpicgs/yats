execute_process(COMMAND "${CMAKE_COMMAND}" --build . --target ${CMAKE_ARGV3} RESULT_VARIABLE res_var ERROR_QUIET)
if("${res_var}" STREQUAL "0")
  message(FATAL_ERROR "Compilation did not fail!")
endif()
