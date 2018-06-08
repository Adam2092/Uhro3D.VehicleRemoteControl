##
# Project testSim
# This is a test of Urho3D project by Yue Kang
# The main purpose is to build up a Docker image combining Urho3D engine and libCluon
#     in order to simulate a simple message-driven vehicle control scenario

project (testSim)

set (TARGET_NAME testRemoteControl)
set (CMAKE_CXX_STANDARD 14)

cmake_minimum_required (VERSION 3.2.3)

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Urho3D/include/Urho3D/ThirdParty)

add_executable(${TARGET_NAME} ${CMAKE_SOURCE_DIR}/src/VehicleRemoteControl.cpp ${CMAKE_SOURCE_DIR}/src/Vehicle.cpp ${CMAKE_SOURCE_DIR}/src/messages.cpp)

# find_library(LIB3D libUrho3D.a ${CMAKE_SOURCE_DIR}/../Urho3D/lib)
# target_link_libraries(${TARGET_NAME} ${LIB3D} pthread dl GL)
target_link_libraries(${TARGET_NAME} ${CMAKE_SOURCE_DIR}/Urho3D/lib/libUrho3D.a pthread dl GL)

add_executable(Listener ${CMAKE_SOURCE_DIR}/src/Listener.cpp ${CMAKE_SOURCE_DIR}/src/messages.cpp)
target_link_libraries(Listener pthread)
