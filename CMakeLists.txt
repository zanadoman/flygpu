cmake_minimum_required(VERSION 4.0)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_COMPILER clang)
file(READ ${CMAKE_SOURCE_DIR}/compile_flags.txt CMAKE_C_FLAGS)
string(REPLACE \n \  CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_DEBUG "-save-temps -Wno-gnu-line-marker")
set(CMAKE_C_FLAGS_RELEASE "-flto -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_CLANG_TIDY clang-tidy;-p;${CMAKE_BINARY_DIR}/)

project(flygpu LANGUAGES C VERSION 0.1.0)
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} ${SOURCES})

find_program(SHADER_COMPILER dxc REQUIRED)
set(SHADER_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_DIR}/)
file(GLOB_RECURSE SHADERS ${CMAKE_SOURCE_DIR}/shaders/*.hlsl)
foreach(SOURCE ${SHADERS})
  get_filename_component(SHADER ${SOURCE} NAME_WLE)
  string(FIND ${SHADER} .vert SHADER_STAGE)
  if (-1 LESS ${SHADER_STAGE})
    set(SHADER_MODEL vs_6_2)
  else()
    string(FIND ${SHADER} .frag SHADER_STAGE)
    if (-1 LESS ${SHADER_STAGE})
      set(SHADER_MODEL ps_6_2)
    else()
      message(FATAL_ERROR "Failed to detect shader stage of ${SOURCE}")
    endif()
  endif()
  set(SPIRV ${SHADER_DIR}/${SHADER}.spv)
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${SHADER_COMPILER} -spirv -T ${SHADER_MODEL} ${SOURCE} -Fo ${SPIRV}
    DEPENDS ${SOURCE}
    VERBATIM
  )
  set(DXIL ${SHADER_DIR}/${SHADER}.dxil)
  add_custom_command(
    OUTPUT ${DXIL}
    COMMAND ${SHADER_COMPILER} -T ${SHADER_MODEL} -Vd ${SOURCE} -Fo ${DXIL}
    DEPENDS ${SOURCE}
    VERBATIM
  )
  add_custom_target(${SHADER} ALL DEPENDS ${SPIRV} ${DXIL})
  add_dependencies(${PROJECT_NAME} ${SHADER})
endforeach()

find_package(SDL3 OPTIONAL CONFIG COMPONENTS SDL3)
find_package(SDL3_image OPTIONAL CONFIG)
if(TARGET SDL3_image::SDL3_image AND TARGET SDL3::SDL3)
  file(GLOB_RECURSE EXAMPLES ${CMAKE_SOURCE_DIR}/examples/*.c)
  foreach(SOURCE ${EXAMPLES})
    get_filename_component(EXAMPLE ${SOURCE} NAME_WLE)
    add_executable(${EXAMPLE} WIN32 ${SOURCE})
    target_link_libraries(
      ${EXAMPLE} PRIVATE SDL3_image::SDL3_image SDL3::SDL3 ${PROJECT_NAME})
    add_custom_command(
      TARGET ${EXAMPLE} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t
        $<TARGET_FILE_DIR:${EXAMPLE}>
        $<TARGET_RUNTIME_DLLS:${EXAMPLE}>
      COMMAND_EXPAND_LISTS
    )
  endforeach()
  file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/assets/)
  file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/)
endif()
