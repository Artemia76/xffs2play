# Necessary CMake
project(XFFS2Play CXX)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter")

set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB GUI gui/*.c*)
file(GLOB NET net/*.c*)
file(GLOB SIM sim/*.c*)
file(GLOB TOOLS tools/*.c*)

file(GLOB_RECURSE MY_HEADERS "*.h*")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(DEBUG_MODE OFF CACHE BOOL "Check this to enable Debug information in plugin logs")

if(DEBUG_MODE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXFFS_DEBUG=1")
endif(DEBUG_MODE)

set(XPLANE_VERSION 11 CACHE STRING "Choose version of targetted X-Plane")

set_property(CACHE XPLANE_VERSION PROPERTY STRINGS 10 11)

if(XPLANE_VERSION EQUAL "11")
	set(XP_VERSION 3)
else()
	set(XP_VERSION 2)
endif()

#Boost dependency

set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

if (APPLE OR WIN32)
  set(Boost_USE_STATIC_LIBS        ON) # only find static libs
  find_package(Boost REQUIRED COMPONENTS system date_time regex)
else ()
  find_package(Boost REQUIRED COMPONENTS system)
endif()

include_directories( ${Boost_INCLUDE_DIR} )

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

#opengl
if (UNIX)
  find_package(GLUT REQUIRED)
  include_directories(${GLUT_INCLUDE_DIR})
endif()

#protobuf

set(PROTOBUF_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

add_subdirectory(proto)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


if( MSVC )
  message(STATUS "Compiler is Microsoft Visual Studio:")
  message(STATUS "  _MSVC_VER is ${MSVC_VERSION} (${ARCH_COMPILER})")

  # globally diable the idiotic min and max macros that cause havoc everywhere
  message(STATUS "  Adding NOMINMAX to compiler defines")
  add_definitions( -DNOMINMAX )

  message(STATUS "  Adding WIN32_LEAN_AND_MEAN to compiler defines")
  add_definitions( -DWIN32_LEAN_AND_MEAN )
endif( MSVC )


add_subdirectory(xplane_sdk/ bin/)

add_xplane_plugin(XFFS2Play
                  ${XP_VERSION}
                  xpmain.cpp
                  ${GUI}
                  ${NET}
                  ${SIM}
                  ${TOOLS}
				  ${MY_HEADERS})

target_link_libraries( XFFS2Play LINK_PUBLIC proto ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY} ${OPENGL_LIBRARIES})

install(TARGETS XFFS2Play DESTINATION Resources/plugins/xffs2play/64)
