cmake_minimum_required(VERSION 3.13)

project(codecrafters-git)

file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.h)

set(CMAKE_C_STANDARD 23) # Enable the C23 standard

add_executable(git ${SOURCE_FILES})

if(WIN32)
    # Configure zlib paths for Windows
    set(ZLIB_ROOT "C:/MinGW")
    set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
    set(ZLIB_LIBRARY "${ZLIB_ROOT}/lib/libz.a")

    include_directories(${ZLIB_INCLUDE_DIR})
    target_link_libraries(git ${ZLIB_LIBRARY})
else()
    # Use find_package for other platforms
    find_package(ZLIB REQUIRED)
    target_link_libraries(git ZLIB::ZLIB)
endif()