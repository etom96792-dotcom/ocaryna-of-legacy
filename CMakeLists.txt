cmake_minimum_required(VERSION 3.22)
project(oml_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# O verificador fica fora de android/ para ser testável no host.
set(VERIFIER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../native/rom_verifier)

add_library(oml_native SHARED
    jni_bridge.cpp
    ${VERIFIER_DIR}/rom_verifier.cpp
    ${VERIFIER_DIR}/sha1.cpp
)
target_include_directories(oml_native PRIVATE ${VERIFIER_DIR})
target_compile_options(oml_native PRIVATE -Wall -Wextra)
