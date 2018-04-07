# Necessary CMake
project(XFFS2Play CXX)
cmake_minimum_required(VERSION 2.8)

file(GLOB GUI gui/*.c*)
file(GLOB NET net/*.c*)
file(GLOB SIM sim/*.c*)
file(GLOB TOOLS tools/*.c*)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#Boost dependency

set(Boost_NO_SYSTEM_PATHS        ON)
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.58.0 REQUIRED COMPONENTS system datetime regex)
include_directories( ${Boost_INCLUDE_DIR} )

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

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
                  3
				  xpmain.cpp
				  ${GUI}
				  ${NET}
				  ${SIM}
				  ${TOOLS})

target_link_libraries( XFFS2Play LINK_PUBLIC ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

install(TARGETS XFFS2Play DESTINATION Resources/plugins/xffs2play/64)
