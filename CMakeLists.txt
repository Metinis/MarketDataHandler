cmake_minimum_required(VERSION 3.16)

project(DataHandler)

add_custom_target(run_all
    COMMAND server &
    COMMAND client
    DEPENDS server client
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SERVER_SOURCES
        server/server_main.cpp
)

set(CLIENT_SOURCES
        client/client_main.cpp
)
find_package(Boost REQUIRED)
add_executable(server server/server_main.cpp
        common/order.h
        common/packet.cpp
        common/server.cpp
        common/server.h
        common/matching_engine.cpp
        common/matching_engine.h
        common/event.h
        common/session.cpp
        common/session.h)

target_link_libraries(server PRIVATE Boost::boost)

add_executable(client client/client_main.cpp
        common/order.h
        common/packet.h
        common/packet.cpp
        common/server.cpp
        common/server.h
        common/matching_engine.cpp
        common/matching_engine.h
        common/event.h
        common/session.cpp
        common/session.h)

target_compile_features(server PUBLIC cxx_std_23)

target_compile_features(client PUBLIC cxx_std_23)


