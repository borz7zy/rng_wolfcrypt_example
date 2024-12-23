cmake_minimum_required(VERSION 3.10)
project(random_lib)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(RANDOM_SOURCES
    random_generator.cpp
)

set(PREBUILDEDLIBS_DIR ${CMAKE_SOURCE_DIR}/prebuilded-libs)
set(wolfssl_root_dir ${PREBUILDEDLIBS_DIR}/wolfssl)
list(APPEND CMAKE_PREFIX_PATH ${wolfssl_root_dir}/lib/cmake/wolfssl)
find_package(Threads REQUIRED)
find_package(wolfssl REQUIRED)

# Create static library
add_library(random_lib STATIC ${RANDOM_SOURCES})

# Include WolfSSL for target random_lib
# target_include_directories(random_lib PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(random_lib PRIVATE wolfssl::wolfssl)

# Set platform-specific defines
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(random_lib PRIVATE -D__linux__)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(random_lib PRIVATE -D__APPLE__)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(random_lib PRIVATE -D_WIN32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_compile_definitions(random_lib PRIVATE -D__ANDROID__)
endif()

# Install library and header
install(TARGETS random_lib
    ARCHIVE DESTINATION lib
)
install(FILES random_generator.h DESTINATION include)
