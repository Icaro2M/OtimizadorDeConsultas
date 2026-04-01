# CMakeList.txt: projeto de CMake para OtimizadorConsultas, incluir origem e definir
# lógica específica a um projeto aqui.
#
cmake_minimum_required (VERSION 3.10)

# Habilite a Recarga Dinâmica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OtimizadorConsultas")

# Adicione a origem ao executável deste projeto.
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"
)

# 2. Cria o executável usando a lista que ele encontrou
add_executable(OtimizadorConsultas ${SOURCES})

# 3. Importante: Diz ao compilador para olhar dentro de 'src' para resolver os #include
target_include_directories(OtimizadorConsultas PRIVATE src)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OtimizadorConsultas PROPERTY CXX_STANDARD 20)
endif()

# TODO: Adicione testes e instale destinos, se necessário.
