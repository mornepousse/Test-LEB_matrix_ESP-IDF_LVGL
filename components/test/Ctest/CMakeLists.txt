cmake_minimum_required(VERSION 3.10)
project(Ctest C)

set(CMAKE_C_STANDARD 11)
include_directories(include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SERIALPORT REQUIRED libserialport)

#add_executable(Ctest src/main.c)

#target_include_directories(Ctest PRIVATE ${SERIALPORT_INCLUDE_DIRS})
#target_link_libraries(Ctest PRIVATE ${SERIALPORT_LIBRARIES})

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(Ctest ${SOURCES})

