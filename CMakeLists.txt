cmake_minimum_required(VERSION 3.28.1)

project(ToyTk CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

add_subdirectory(libtoytk)
add_subdirectory(demo)

install(EXPORT ToyTk
  DESTINATION share/toytk/cmake
  NAMESPACE ToyTk::)
