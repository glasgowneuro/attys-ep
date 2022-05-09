cmake_minimum_required(VERSION 3.7.0)

project(attys-ep VERSION 1.1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Multimedia REQUIRED)

add_executable(attys-ep
  attys-ep.cpp
  vepplot.cpp 
  dataplot.cpp 
  main.cpp 
  stim.cpp
  audiobeep.cpp
  application.qrc
)

target_link_libraries(attys-ep Qt5::Widgets)
target_link_libraries(attys-ep Qt5::Multimedia)

if(CMAKE_HOST_SYSTEM MATCHES Linux)
  message(STATUS "Build host runs Linux")
  include_directories(/usr/include/qwt)
  target_link_libraries(attys-ep iir attyscomm bluetooth qwt-qt5)
endif()

if(CMAKE_SYSTEM MATCHES Windows)
   message(STATUS "Target system is Windows")
endif()

install(TARGETS attys-ep)
