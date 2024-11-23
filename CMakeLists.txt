cmake_minimum_required(VERSION 3.13)

project(codecrafters-git)

file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.h)

set(CMAKE_C_STANDARD 23) # Enable the C23 standard

add_executable(git ${SOURCE_FILES}
        src/utils/file/file.h
        src/utils/file/file.c
        src/utils/string/string.h
        src/utils/string/string.c
        src/utils/compression/compression.h
        src/utils/compression/compression.c
        src/commands/commands.h
        src/constants.h
        src/commands/commands.c)

if(WIN32)
    # Configure zlib paths for Windows
    set(ZLIB_ROOT "C:/MinGW")
    set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
    set(ZLIB_LIBRARY "${ZLIB_ROOT}/lib/libz.a")

    include_directories(${ZLIB_INCLUDE_DIR})
    target_link_libraries(git ${ZLIB_LIBRARY})

    # Add OpenSSL paths
    set(OPENSSL_ROOT "C:/Program Files (x86)/OpenSSL-Win32") # Adjust to your OpenSSL installation path
    include_directories("${OPENSSL_ROOT}/include")
    link_directories("${OPENSSL_ROOT}/lib")

    # Link OpenSSL libraries
    target_link_libraries(git libcrypto) # Add OpenSSL's libcrypto
else()
    # Use find_package for other platforms
    find_package(ZLIB REQUIRED)
    target_link_libraries(git ZLIB::ZLIB)

    find_package(OpenSSL REQUIRED)
    target_link_libraries(git OpenSSL::Crypto) # Link OpenSSL on non-Windows systems
endif()