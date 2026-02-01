# cmake/CneriumWarnings.cmake
# Warning presets (portable, strict, but not insane).

include_guard(GLOBAL)

function(cnerium_apply_warnings target_name)
  if(NOT TARGET ${target_name})
    message(FATAL_ERROR "cnerium_apply_warnings: target '${target_name}' not found")
  endif()

  if(MSVC)
    target_compile_options(${target_name} PRIVATE
      /W4
      /permissive-
      /Zc:__cplusplus
    )
    if(CNERIUM_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target_name} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Woverloaded-virtual
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
    )
    if(CNERIUM_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE -Werror)
    endif()
  endif()
endfunction()
