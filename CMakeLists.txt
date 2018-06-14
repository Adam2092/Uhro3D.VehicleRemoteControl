##
# Project testSim
# This is a test of Urho3D project by Yue Kang	+# This is a test of Urho3D project building by Yue Kang
# The main purpose is to build up a Docker image combining Urho3D engine and libCluon	+# The purpose of this test project is to find out the possibility and
#     in order to simulate a simple message-driven vehicle control scenario

project (testSim)

set (TARGET_NAME testRemoteControl)
set (CMAKE_CXX_STANDARD 14)

# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# So far, every warning observed is caused by code from Urho3D itself
# Same reason that unfortunately we can not use "-Werror" here

cmake_minimum_required (VERSION 3.2.3)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/Urho3D/include)
include_directories(${CMAKE_SOURCE_DIR}/Urho3D/include/Urho3D/ThirdParty)

## The following part (generating cluon-based .hpp and .cpp files) are integrated into script file.
#find_package(Threads REQUIRED)
#add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cluon-msc
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/${CLUON_COMPLETE} ${CMAKE_BINARY_DIR}/cluon-complete.hpp
#    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/cluon-complete.hpp ${CMAKE_BINARY_DIR}/cluon-complete.cpp
#    COMMAND ${CMAKE_CXX_COMPILER} -o ${CMAKE_BINARY_DIR}/cluon-msc ${CMAKE_BINARY_DIR}/cluon-complete.cpp -std=c++14 -pthread -D HAVE_CLUON_MSC
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${CLUON_COMPLETE})

#set (OPENDLV_STANDARD_MESSAGE_SET messages.odvd)

# add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/messages.cpp
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    COMMAND ${CMAKE_BINARY_DIR}/cluon-msc --cpp-sources --cpp-add-include-file=messages.hpp --out=${CMAKE_BINARY_DIR}/messages.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${OPENDLV_STANDARD_MESSAGE_SET}
#    COMMAND ${CMAKE_BINARY_DIR}/cluon-msc --cpp-headers --out=${CMAKE_BINARY_DIR}/messages.hpp ${CMAKE_CURRENT_SOURCE_DIR}/${OPENDLV_STANDARD_MESSAGE_SET}
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${OPENDLV_STANDARD_MESSAGE_SET} ${CMAKE_BINARY_DIR}/cluon-msc)

# add_executable(${TARGET_NAME} ${CMAKE_SOURCE_DIR}/VehicleDemo.cpp ${CMAKE_SOURCE_DIR}/Vehicle.cpp ${CMAKE_SOURCE_DIR}/messages.cpp ${CMAKE_SOURCE_DIR}/libUrho3D.a)

add_executable(${TARGET_NAME} ${CMAKE_SOURCE_DIR}/src/VehicleRemoteControl.cpp ${CMAKE_SOURCE_DIR}/src/Vehicle.cpp ${CMAKE_SOURCE_DIR}/src/messages.cpp)

find_library(LIB3D libUrho3D.a ${CMAKE_SOURCE_DIR}/Urho3D/lib)
target_link_libraries(${TARGET_NAME} ${LIB3D} pthread dl GL)

add_executable(Listener ${CMAKE_CURRENT_SOURCE_DIR}/src/Listener.cpp ${CMAKE_SOURCE_DIR}/src/messages.cpp)
target_link_libraries(Listener pthread)
