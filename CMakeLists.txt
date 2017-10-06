cmake_minimum_required(VERSION 3.8)
project(libcppweb)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.64 REQUIRED COMPONENTS system thread)

add_library(libcppweb STATIC src/async_connect.cpp)
target_include_directories(libcppweb PUBLIC include)

add_executable(testcppweb test/main.cpp)

target_link_libraries(testcppweb libcppweb)
target_link_libraries(testcppweb Boost::system Boost::thread)
