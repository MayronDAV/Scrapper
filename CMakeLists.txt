cmake_minimum_required(VERSION 3.16)

project(Scrapper LANGUAGES C CXX)

if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(FATAL_ERROR "Compilation inside source folder not allowed. Use a build folder.")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT MSVC)
	# globally set -fno-strict-aliasing, needed due to using reinterpret_cast
  	add_compile_options(-fno-strict-aliasing)
endif()

if(MSVC)
	add_compile_options(/std:c++20 /MP)
	add_link_options(/ignore:4099)
endif()

# create bin folder
file(MAKE_DIRECTORY bin)

set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}/lib)

if (LINUX AND NOT APPLE)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}/${CMAKE_BUILD_TYPE})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR}/lib/${CMAKE_BUILD_TYPE})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}/lib/${CMAKE_BUILD_TYPE})
endif()

if(WIN32)
	set(VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

include_directories(
	Source
	${GLAD_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${CPR_INCLUDE_DIR}
	${LEXBOR_INCLUDE_DIR}
)

add_subdirectory(Thirdparty)

file(GLOB_RECURSE SCRAPPER_SOURCES "Source/**.cpp" "Source/**.h")
add_executable(Scrapper ${SCRAPPER_SOURCES})
target_link_libraries(Scrapper PRIVATE ${OPENGL_LIBS} glad glfw imgui cpr lexbor_static)