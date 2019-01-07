cmake_minimum_required(VERSION 3.6)
project(GlslToSpv)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive -fvisibility=hidden -Wall -std=c++14" )
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        glsl_to_spv.cpp
        global_fun.cpp
        )

add_executable(glsl-to-spv ${SOURCE_FILES})

target_include_directories(glsl-to-spv PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_SOURCE_DIR}/../shaderc/libshaderc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../shaderc/third_party/spirv-tools/external/SPIRV-Headers/include/spirv/1.2
        )

target_link_libraries(glsl-to-spv
        shaderc
        shaderc_util
        glslang
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt
        OSDependent
        OGLCompiler)


#target_link_libraries()