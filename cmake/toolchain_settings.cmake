if(NOT CMAKE_TOOLCHAIN_FILE)
message(
    STATUS "Setting toolchain type to build using mscv.")
  set(CMAKE_TOOLCHAIN_FILE
  ${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake
     CACHE STRING "Choose the type of build." FORCE)
  #set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/vcpkg/installed/x64-windows/)
  message(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
  message(CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}")
  message(CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}")

endif()

set(CMAKE_CXX_EXTENSIONS OFF)