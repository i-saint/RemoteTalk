CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(RemoteTalk)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Poco COMPONENTS Net QUIET)
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    mark_as_advanced(COCOA_LIBRARY)
    set(PLATFORM "Mac")
    set(MAC ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(PLATFORM "Linux")
    set(LINUX ON)
endif()

include(AddPlugin)

# Client
option(BUILD_CLIENT "Build RemoteTalkClient" ON)

# Tests
option(BUILD_TESTS "Tests" OFF)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fPIC -std=c++14 -Wno-deprecated")

function(add_subdirectory_link to from)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${to}")
        execute_process(WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" COMMAND ln -s "${from}" "${to}")
    endif()
    add_subdirectory("${to}")
endfunction()

add_subdirectory(RemoteTalk)

# Client
if(BUILD_CLIENT)
    add_subdirectory(RemoteTalkClient)
endif()

# Tests
if(BUILD_TESTS)
    add_subdirectory(Test)
endif()

