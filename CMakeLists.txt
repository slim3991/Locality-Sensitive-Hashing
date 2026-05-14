cmake_minimum_required(VERSION 3.14) # Required for FetchContent_MakeAvailable
project(LSH_Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 1. Fetch Catch2 from GitHub ---
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # Or the latest stable version
)
FetchContent_MakeAvailable(Catch2)

# --- 2. Main Application ---
add_executable(lsh_app src/main.cpp)

# --- 3. Test Executable ---
# Assuming your tests will be in a new file: tests/test_lsh.cpp
add_executable(lsh_tests tests/test_lsh.cpp)

# Link the test executable to Catch2
# Catch2::Catch2WithMain provides a default main() for your tests
target_link_libraries(lsh_tests PRIVATE Catch2::Catch2WithMain)

# Include the source directory so the test file can find your LSH class
# (If LSH is moved to a header file, which is recommended)
target_include_directories(lsh_tests PRIVATE src)
