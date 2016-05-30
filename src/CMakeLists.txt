cmake_minimum_required(VERSION 2.8.11)
cmake_policy(SET CMP0028 NEW)

project(dbe)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-fPIC)
add_definitions(-std=c++11)

find_package(Qt5Widgets REQUIRED)
find_package(PythonLibs 3 REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

add_subdirectory(widgets)

add_executable(dbe dbe.cpp $<TARGET_OBJECTS:widgets>)

# Allow building Qt and python together
target_compile_definitions(dbe PRIVATE QT_NO_KEYWORDS)

target_link_libraries(dbe Qt5::Widgets ${PYTHON_LIBRARIES})