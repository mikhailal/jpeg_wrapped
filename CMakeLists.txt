# Builds shared-lib _jpeg_manipulator.so and installs to ./python dir

cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)
include(UseSWIG)
include(FindBoost)

cmake_minimum_required(VERSION 3.16)
project(jpgmanip_project)
find_package(Boost REQUIRED)
find_package(JPEG REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(SWIG COMPONENTS python)
set_property(SOURCE ${CMAKE_SOURCE_DIR}/swig/jpeg_manipulator.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE ${CMAKE_SOURCE_DIR}/swig/jpeg_manipulator.i PROPERTY SWIG_MODULE_NAME jpeg_manipulator)
swig_add_library(jpeg_manipulator
        TYPE SHARED
        LANGUAGE PYTHON
        OUTPUT_DIR ${CMAKE_SOURCE_DIR}/python
        SOURCES ${CMAKE_SOURCE_DIR}/src/jpeg_manipulator.cpp ${CMAKE_SOURCE_DIR}/swig/jpeg_manipulator.i)

include_directories(${Python3_INCLUDE_DIRS})
set(JMANIP_INTERFACE "./interface")
include_directories(jpeg_manipulator ${JMANIP_INTERFACE})

swig_link_libraries(jpeg_manipulator ${Python3_LIBRARIES})
swig_link_libraries(jpeg_manipulator ${JPEG_LIBRARIES})
add_compile_options(-g -Werror)
add_executable(testapp ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(testapp jpeg_manipulator)
#workaround
if( (${CMAKE_SYSTEM_NAME} STREQUAL "Linux") AND
        (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") AND
        (${CMAKE_COMPILER_IS_GNUCXX}))
    link_directories(${Boost_LIBRARY_DIRS}/x86_64-linux-gnu/)
    endif()
target_link_libraries(testapp boost_program_options)
install(TARGETS jpeg_manipulator DESTINATION ${CMAKE_SOURCE_DIR}/python)
