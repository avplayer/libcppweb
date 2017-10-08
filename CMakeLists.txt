cmake_minimum_required(VERSION 3.8)
project(libcppweb)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.64 REQUIRED COMPONENTS system thread regex)

add_library(libcppweb STATIC src/proxy_error.cpp)
target_include_directories(libcppweb PUBLIC include)
target_link_libraries(libcppweb Boost::system Boost::thread Boost::regex)

add_executable(testcppweb test/main.cpp)

target_link_libraries(testcppweb libcppweb)
target_link_libraries(testcppweb Boost::system Boost::thread)
