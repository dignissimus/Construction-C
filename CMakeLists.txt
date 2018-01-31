cmake_minimum_required(VERSION 3.8)
project(Construction)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Libedit REQUIRED)
include_directories(${Libedit_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Libedit_LIBRARIES})

find_package(LLVM REQUIRED)
include_directories(${LLVM_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LLVM_LIBRARIES})

set(SOURCE_FILES main util mpc)
add_executable(Construction ${SOURCE_FILES})
target_link_libraries(Construction ${LIBEDIT_LIBRARIES} ${LLVM_LIBRARIES} m)
file(COPY lib-construction DESTINATION ${CMAKE_BINARY_DIR})
