function(patch_include_directories TARGET)
  target_include_directories(${TARGET}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

function(prepend VAR PREFIX)
  set(LIST_VAR "")
  foreach(ELEM ${ARGN})
    list(APPEND LIST_VAR "${PREFIX}/${ELEM}")
  endforeach()
  set(${VAR} "${LIST_VAR}" PARENT_SCOPE)
endfunction()

function(add_task_executable NAME)
  set(MULTI_VALUE_ARGS SOLUTION_SRCS)
  cmake_parse_arguments(SHAD_LIBRARY "" "" "${MULTI_VALUE_ARGS}" ${ARGN})

  get_filename_component(TASK_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

  add_executable(${NAME}
    ${SHAD_LIBRARY_UNPARSED_ARGUMENTS}
    ${SHAD_LIBRARY_SOLUTION_SRCS})

  patch_include_directories(${NAME})
endfunction()

add_custom_target(test-all)

function(add_catch TARGET)
  add_task_executable(${TARGET}
    ${ARGN})

  target_link_libraries(${TARGET}
    contrib_catch_main)
endfunction()
