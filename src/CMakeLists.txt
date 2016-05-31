cmake_minimum_required(VERSION 2.8.11)
cmake_policy(SET CMP0028 NEW)

project(dbe)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GetGitRevisionDescription)

git_describe(DBE_VERSION --tags --long)

configure_file (
  "${PROJECT_SOURCE_DIR}/dbe_info.hpp.in"
  "${PROJECT_BINARY_DIR}/dbe_info.hpp"
  )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# position-independent code (should work find_package on Qt5Widgets)
add_definitions(-fPIC)

# C++11
add_definitions(-std=c++11)

# Allow building Qt and python together
add_definitions(-DQT_NO_KEYWORDS)

find_package(Qt5Widgets REQUIRED)
find_package(PythonLibs 3 REQUIRED)

include_directories("${PROJECT_BINARY_DIR}")
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

add_subdirectory(widgets)

add_executable(dbe dbe.cpp $<TARGET_OBJECTS:widgets>)



target_link_libraries(dbe Qt5::Widgets ${PYTHON_LIBRARIES})
