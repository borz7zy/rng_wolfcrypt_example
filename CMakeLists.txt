cmake_minimum_required(VERSION 3.10)
project(random_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the subdirectory for random_lib
add_subdirectory(random_lib)

set(PREBUILDEDLIBS_DIR ${CMAKE_SOURCE_DIR}/prebuilded-libs)
set(wolfssl_root_dir ${PREBUILDEDLIBS_DIR}/wolfssl)
list(APPEND CMAKE_PREFIX_PATH ${wolfssl_root_dir}/lib/cmake/wolfssl)
find_package(Threads REQUIRED)
find_package(wolfssl REQUIRED)

set(random_test_src
    random_test.cpp
)

# Create the executable
add_executable(random_test ${random_test_src})

# Link the random_lib to the executable
target_link_libraries(random_test PRIVATE random_lib wolfssl::wolfssl)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(random_test PRIVATE -D__linux__ )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_definitions(random_test PRIVATE -D__APPLE__ )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(random_test PRIVATE -D_WIN32 )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  target_compile_definitions(random_test PRIVATE -D__ANDROID__)
endif()