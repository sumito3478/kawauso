cmake_minimum_required(VERSION 3.0.0)
project(kawauso)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

add_executable(
  kawauso
  main.cpp
  MainWindow.cpp
)

target_link_libraries(
  kawauso
  Qt5::Widgets
)

# vim:set sw=2 ts=2 et:
