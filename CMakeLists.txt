# Necessary CMake
project(XFFS2Play CXX)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

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

if (APPLE)
	find_library(corefoundation_lib CoreFoundation)
	find_library(cfnetwork_lib CFNetwork)
	set(frameworks
    		${cfnetwork_lib}
    		${corefoundation_lib})
endif()

#Boost dependency
if (APPLE OR WIN32)
  set(Boost_USE_STATIC_LIBS        ON) # only find static libs
  find_package(Boost REQUIRED COMPONENTS thread system date_time regex)
else ()
  find_package(Boost REQUIRED COMPONENTS system)
endif()
if (Boost_FOUND)
	set(Boost_USE_MULTITHREADED      ON)
	set(Boost_USE_STATIC_RUNTIME    OFF)

	include_directories( ${Boost_INCLUDE_DIR} )
endif ( Boost_FOUND )

#opengl
find_package(OpenGL REQUIRED)
if ( OpenGL_FOUND )
	include_directories(${OPENGL_INCLUDE_DIR})
endif ( OpenGL_FOUND )
#glut
if (UNIX)
	find_package(GLUT REQUIRED)
	if ( GLUT_FOUND )
		include_directories(${GLUT_INCLUDE_DIR})
	endif( GLUT_FOUND )
endif()

#protobuf

find_package(Protobuf REQUIRED)
if ( Protobuf_FOUND )
	set(PROTOBUF_USE_STATIC_LIBS ON)
	include_directories(${PROTOBUF_INCLUDE_DIRS})
endif ( Protobuf_FOUND )

add_subdirectory(proto)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#ZLIB
find_package(ZLIB REQUIRED)  # sudo apt-get install zlib1g-dev
if ( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif( ZLIB_FOUND )

#libpng
find_package(PNG REQUIRED)  # sudo apt-get install libpng-dev
if ( PNG_FOUND )
    include_directories( ${PNG_INCLUDE_DIRS} )
endif ( PNG_FOUND )

#xplanemp
set ( XPSDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xplane_sdk/sdk/")
add_subdirectory (libxplanemp)
include_directories( libxplanemp/include )

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

target_link_libraries( XFFS2Play LINK_PUBLIC proto xplanemp ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY} ${OPENGL_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${frameworks})

install(TARGETS XFFS2Play DESTINATION Resources/plugins/xffs2play/64)
