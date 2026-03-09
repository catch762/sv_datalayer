cmake_minimum_required(VERSION 3.16)
project(sv_datalayer LANGUAGES CXX)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME ordered-map
    GIT_REPOSITORY git@github.com:Tessil/ordered-map.git
    GIT_TAG master
)

CPMAddPackage(
  NAME Boost
  VERSION 1.89.0
  URL https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.xz
  OPTIONS
    "BOOST_ENABLE_CMAKE ON"
    "BUILD_SHARED_LIBS OFF"
    "BOOST_INCLUDE_LIBRARIES bimap"
)

file(GLOB_RECURSE SV_DATALAYER_SOURCES "*.cpp" "*.h")

add_library(sv_datalayer STATIC ${SV_DATALAYER_SOURCES})

target_include_directories(sv_datalayer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sv_datalayer PUBLIC sv_qtcommon tsl::ordered_map Boost::bimap)