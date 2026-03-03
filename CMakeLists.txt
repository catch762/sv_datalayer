cmake_minimum_required(VERSION 3.16)
project(sv_datalayer LANGUAGES CXX)

file(GLOB_RECURSE SV_DATALAYER_SOURCES "*.cpp" "*.h")

add_library(sv_datalayer STATIC ${SV_QTCOMMON_SOURCES})

target_include_directories(sv_datalayer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sv_datalayer PRIVATE sv_qtcommon)