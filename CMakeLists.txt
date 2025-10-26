cmake_minimum_required(VERSION 3.15)
project(SHA256Test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Directories ---
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Library target ---
add_library(sha256
    src/sha256.cpp
)

# --- Test executable ---
add_executable(test_sha256
    tests/test_sha256.cpp
)

# --- Link SHA256 library to tests ---
target_link_libraries(test_sha256 PRIVATE sha256)

# --- Compiler warnings ---
target_compile_options(sha256 PRIVATE -Wall -Wextra -pedantic)
target_compile_options(test_sha256 PRIVATE -Wall -Wextra -pedantic)

# --- Optional: AddressSanitizer (use: cmake -DENABLE_ASAN=ON ..) ---
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if (ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    foreach(target sha256 test_sha256)
        target_compile_options(${target} PRIVATE -fsanitize=address -g)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endforeach()
endif()

# --- Add a 'run_tests' target to build & execute automatically ---
add_custom_target(run_tests
    COMMAND test_sha256
    DEPENDS test_sha256
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running SHA-256 test suite..."
)
