cmake_minimum_required(VERSION 3.14)

project(aseman-remote-tester LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(aseman-remote-tester
  main.cpp
  socketengine.h
  socketengine.cpp
  aseman/asemansocketinterface.cpp
  aseman/asemansocketinterface.h
  aseman/artserver.cpp
  aseman/artserver.h
  aseman/artclient.cpp
  aseman/artclient.h
  testserver.h testserver.cpp
  testclient.h testclient.cpp
  abstracttest.h abstracttest.cpp
  testserversession.h testserversession.cpp
  filewatcher.h filewatcher.cpp
  testclientsession.h testclientsession.cpp
)
target_link_libraries(aseman-remote-tester Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

include(GNUInstallDirs)
install(TARGETS aseman-remote-tester
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
