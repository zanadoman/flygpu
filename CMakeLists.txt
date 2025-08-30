cmake_minimum_required(VERSION 4.0.3)

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
target_link_libraries(${PROJECT_NAME} SDL3)

set(SHADER_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_DIRECTORY}/)
file(GLOB_RECURSE SHADERS ${CMAKE_SOURCE_DIR}/shaders/*.hlsl)
foreach(SOURCE ${SHADERS})
  get_filename_component(SHADER ${SOURCE} NAME_WLE)
  set(SPIRV ${SHADER_DIRECTORY}/${SHADER}.spv)
  set(DXIL ${SHADER_DIRECTORY}/${SHADER}.dxil)
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND dxc -spirv -T lib_6_8 ${SOURCE} -Fo ${SPIRV}
    DEPENDS ${SOURCE}
    VERBATIM
  )
  add_custom_command(
    OUTPUT ${DXIL}
    COMMAND dxc -T lib_6_8 ${SOURCE} -Fo ${DXIL}
    DEPENDS ${SOURCE}
    VERBATIM
  )
  add_custom_target(${SHADER} ALL DEPENDS ${SPIRV} ${DXIL})
  add_dependencies(${PROJECT_NAME} ${SHADER})
endforeach()

file(GLOB_RECURSE EXAMPLES ${CMAKE_SOURCE_DIR}/examples/*.c)
foreach(SOURCE ${EXAMPLES})
  get_filename_component(EXAMPLE ${SOURCE} NAME_WLE)
  add_executable(${EXAMPLE} WIN32 ${SOURCE})
  target_link_libraries(${EXAMPLE} SDL3_image ${PROJECT_NAME})
endforeach()

file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/assets/)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/)
