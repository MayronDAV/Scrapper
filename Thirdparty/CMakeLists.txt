cmake_minimum_required(VERSION 3.10)

project(Thirdparty LANGUAGES C CXX)

set(D ${CMAKE_CURRENT_SOURCE_DIR})

set(OPENGL_LIBS )

if(WIN32)
	list(APPEND OPENGL_LIBS opengl32)
  	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
	find_package(OpenGL REQUIRED)
	add_definitions(${OPENGL_DEFINITIONS})
	list(APPEND OPENGL_LIBS GL)
elseif(APPLE)
	set(FIND_PATH "/System/Library/Frameworks")
	find_library(OpenGL_LIBRARY OpenGL PATHS FIND_PATH)
	mark_as_advanced(OpenGL_LIBRARY)
	list(APPEND OPENGL_LIBS ${OpenGL_LIBRARY})
endif()

# GLAD
set(GLAD_DIR ${D}/glad)
set(GLAD_INCLUDE_DIR ${GLAD_DIR}/include)
add_library(glad INTERFACE)
target_sources(glad INTERFACE ${GLAD_DIR}/src/glad.c)
target_include_directories(glad SYSTEM INTERFACE ${GLAD_INCLUDE_DIR})
set_property(TARGET glad PROPERTY FOLDER "Thirdparty")

# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)
option(BUILD_SHARED_LIBS ON)
if (VKB_WSI_SELECTION STREQUAL WAYLAND)
    set(GLFW_USE_WAYLAND ON)
endif()
set(GLFW_INCLUDE_DIR ${D}/glfw/include)
add_subdirectory(glfw)
if (TARGET update_mappings)
    set_target_properties(update_mappings PROPERTIES FOLDER "Thirdparty")
endif()
set_property(TARGET glfw PROPERTY FOLDER "Thirdparty")

# IMGUI
file(GLOB IMGUI_FILES ${D}/imgui/*.cpp ${D}/imgui/*.h)
set(IMGUI_INCLUDE_DIR ${D}/imgui)
list(APPEND IMGUI_FILES ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_glfw.h;${IMGUI_INCLUDE_DIR}/backends/imgui_impl_glfw.cpp)
list(APPEND IMGUI_FILES ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_opengl3_loader.h;${IMGUI_INCLUDE_DIR}/backends/imgui_impl_opengl3.h;${IMGUI_INCLUDE_DIR}/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_FILES})
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
target_link_libraries(imgui PRIVATE glfw)
target_link_libraries(imgui PRIVATE ${OPENGL_LIBS})
set_property(TARGET imgui PROPERTY FOLDER "Thirdparty")

# CPR
set(CPR_INCLUDE_DIR ${D}/cpr/include)
set(CPR_BUILD_TESTS OFF)
add_subdirectory(cpr)
set_property(TARGET cpr PROPERTY FOLDER "Thirdparty")

# LEXBOR
set(LEXBOR_INCLUDE_DIR ${D}/lexbor/include)
set(LEXBOR_BUILD_SHARED OFF)
set(LEXBOR_BUILD_STATIC ON)
add_subdirectory(lexbor)
set_property(TARGET lexbor_static PROPERTY FOLDER "Thirdparty")

include(FetchContent)

# YAML
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)
set_property(TARGET yaml-cpp PROPERTY FOLDER "Thirdparty")

# UTF8CPP 
FetchContent_Declare(
    utf8cpp
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
    GIT_TAG v4.0.6
)
FetchContent_MakeAvailable(utf8cpp )
set_property(TARGET utf8cpp  PROPERTY FOLDER "Thirdparty")