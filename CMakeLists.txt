cmake_minimum_required(VERSION 3.31)
project(cppPythonStorage)

set(CMAKE_CXX_STANDARD 17)

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

add_executable(cppPythonStorage src/main.cpp
        src/factorWriter.cpp
        src/factorWriter.h
        src/factorWriterGZ.cpp
        src/factorWriterGZ.h)

target_link_libraries(cppPythonStorage PRIVATE ${ZLIB_LIBRARIES})