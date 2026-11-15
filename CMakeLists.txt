cmake_minimum_required(VERSION 3.12)

project(SimpleHttpServer
    DESCRIPTION "A simple web server that supports HTTP/1.1"
    LANGUAGES CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

include_directories(src)

set(SRC_DIR src)
set(TEST_DIR test)

add_executable(SimpleHttpServer
    ${SRC_DIR}/main.cc
    ${SRC_DIR}/http_server.cc
    ${SRC_DIR}/http_message.cc
    ${SRC_DIR}/query_utils.cc
    ${SRC_DIR}/weather_handler.cc
    ${SRC_DIR}/weather_api.cc
    ${SRC_DIR}/json.cc
    ${SRC_DIR}/weather_parser.cc
)

add_executable(test_SimpleHttpServer
    ${TEST_DIR}/main.cc
    ${SRC_DIR}/http_server.cc
    ${SRC_DIR}/http_message.cc
)

target_link_libraries(SimpleHttpServer PRIVATE Threads::Threads CURL::libcurl)
target_link_libraries(test_SimpleHttpServer PRIVATE Threads::Threads)
