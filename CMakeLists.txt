cmake_minimum_required(VERSION 3.15)
project(MeuProjeto CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV
find_package(OpenCV REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

# Flags de compilação
add_compile_options(-Wall -Wextra -g3)

# Saída dos executáveis
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

# Arquivo executável principal
add_executable(app
    src/main.cpp
    src/solucaoA.cpp
    src/solucaoB.cpp
    src/solucaoC.cpp
    src/utils.cpp
)

# Link OpenCV
target_link_libraries(app ${OpenCV_LIBS})