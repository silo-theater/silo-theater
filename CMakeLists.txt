cmake_minimum_required(VERSION 3.21)

project(silo-theater VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Builds the Amlogic decode, display and audio code. Off on a desktop, where
# the UI and backends run without the hardware video path.
option(SILO_AML "Build the Amlogic player path (libsilo-aml)" OFF)
option(BUILD_TESTING "Build the tests" ON)

if(SILO_AML)
  message(FATAL_ERROR "SILO_AML=ON: libsilo-aml has not been extracted yet")
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Gui Qml Quick)
qt_standard_project_setup()

add_subdirectory(src)

if(BUILD_TESTING)
  enable_testing()
  find_package(Qt6 REQUIRED COMPONENTS Test)
  add_subdirectory(tests)
endif()
