cmake_minimum_required (VERSION 3.10)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("OtimizadorConsultas")

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"
)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_subdirectory(vendor/glfw)

add_executable(OtimizadorConsultas
    ${SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(OtimizadorConsultas PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(OtimizadorConsultas PRIVATE glfw)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET OtimizadorConsultas PROPERTY CXX_STANDARD 20)
endif()

find_package(OpenGL REQUIRED)

target_link_libraries(OtimizadorConsultas
    PRIVATE
    OpenGL::GL
)