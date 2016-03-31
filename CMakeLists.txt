cmake_minimum_required(VERSION 2.6)
project(luapp)

option(build_tests "build luapp tests" ON)

if(NOT CMAKE_BUILD_BITS)
    set(CMAKE_BUILD_BITS -m32)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE -O3)

if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
endif()

find_path(lua_dir lua.hpp)
find_library(liblua NAMES lua)

include_directories(${lua_dir})
aux_source_directory(./src luapp_src_dir)

add_library(luapp SHARED ${luapp_src_dir})
set_target_properties(luapp PROPERTIES VERSION 1.0)

if(build_tests)
    include_directories(src test)
    add_library(example SHARED
        "./test/example.cpp"
        "./test/luagen/example.cpp"
    )
    target_link_libraries(example luapp)

    find_library(libdl NAMES dl)
    add_executable(test_luapp "./test/main.cpp")
    target_link_libraries(test_luapp example luapp ${liblua} ${libdl})
endif()

set(CMAKE_INSTALL_PREFIX /usr/local)

install(TARGETS luapp DESTINATION lib)
install(DIRECTORY ./src/ DESTINATION include/luapp
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN "aes.hpp" EXCLUDE
    PATTERN "class_meta.hpp" EXCLUDE
)
