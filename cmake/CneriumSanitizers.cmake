# cmake/CneriumSanitizers.cmake
# Sanitizers (best-effort). Avoid enabling multiple incompatible sanitizers.

include_guard(GLOBAL)

function(cnerium_apply_sanitizers target_name)
  if(NOT TARGET ${target_name})
    message(FATAL_ERROR "cnerium_apply_sanitizers: target '${target_name}' not found")
  endif()

  if(MSVC)
    # MSVC sanitizers exist but differ; keep disabled by default for portability.
    return()
  endif()

  if(CNERIUM_ENABLE_TSAN AND CNERIUM_ENABLE_SANITIZERS)
    message(FATAL_ERROR "Enable either TSAN or ASAN/UBSAN, not both")
  endif()

  if(CNERIUM_ENABLE_TSAN)
    target_compile_options(${target_name} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
    target_link_options(${target_name} PRIVATE -fsanitize=thread)
  endif()

  if(CNERIUM_ENABLE_SANITIZERS)
    target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
  endif()
endfunction()
