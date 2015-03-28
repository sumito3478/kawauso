cmake_minimum_required(VERSION 3.0.0)
project(kawauso)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/fakevim/fakevim
  ${CMAKE_CURRENT_SOURCE_DIR}/fakevim/fakevim/utils
)

add_definitions(-DFAKEVIM_STANDALONE)

find_package(Qt5Widgets)

add_executable(
  kawauso
  main.cpp
  MainWindow.cpp
  VimEditor.cpp
  fakevim/fakevim/fakevimhandler.cpp
  fakevim/fakevim/fakevimactions.cpp
  fakevim/fakevim/utils/qtcassert.cpp
)

target_link_libraries(
  kawauso
  Qt5::Widgets
)

# vim:set sw=2 ts=2 et:
