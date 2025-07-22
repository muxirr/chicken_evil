cmake_minimum_required(VERSION 3.25)

project(chicken_evil)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

include_directories(include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  source/*.cpp
  source/*.c)

if(MSVC OR MINGW)
  list(APPEND SOURCES source/resource.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
  SDL2::SDL2
  $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
  $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
  $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)

if(MSVC AND (NOT CMAKE_BUILD_TYPE STREQUAL "Debug"))
  target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
elseif(MINGW AND (NOT CMAKE_BUILD_TYPE STREQUAL "Debug"))
  target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})