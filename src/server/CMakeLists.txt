cmake_minimum_required(VERSION 3.12)
project(ChatServer VERSION 1.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Boost 1.66 REQUIRED COMPONENTS system)


add_executable(chat_server
    cpp/server.cpp
    cpp/server_main.cpp
    
)


target_link_libraries(chat_server
    PRIVATE
        Boost::system
        pthread
)

install(TARGETS chat_server DESTINATION bin)