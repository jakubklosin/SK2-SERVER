cmake_minimum_required(VERSION 3.12)
project(ServerApp)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/include)

add_executable(server server2poll.cpp)
