# XFFS2Play - X-Plane plugin linking X-Plane to the FFS2Play multiplayer client
#
# Dependencies come from vcpkg (see vcpkg.json and CMakePresets.json);
# XPMP2 is a git submodule and also provides the X-Plane SDK (XPMP2/lib/SDK).
#
#   cmake --preset windows-msvc
#   cmake --build --preset windows-msvc
#   cmake --install build/windows-msvc --config Release --prefix "<X-Plane>/Resources/plugins"

cmake_minimum_required(VERSION 3.23)

project(XFFS2Play
        VERSION 2.1.0
        DESCRIPTION "X-Plane plugin linking X-Plane to the FFS2Play multiplayer client"
        HOMEPAGE_URL "https://github.com/Artemia76/xffs2play"
        LANGUAGES CXX)

option(XFFS_DEBUG "Write verbose debug messages to X-Plane's Log.txt" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
if(APPLE)
    # Minimum system requirement of X-Plane 12
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
endif()

################################################################################
# Dependencies
################################################################################

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# XPMP2 and the X-Plane SDK it ships
set(XPlaneSDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/XPMP2/lib/SDK")
add_subdirectory(XPMP2 EXCLUDE_FROM_ALL)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/XPMP2/cmake")
find_package(XPlaneSDK REQUIRED OPTIONAL_COMPONENTS XPLM)

add_subdirectory(proto)

################################################################################
# Plugin
################################################################################

configure_file(tools/version.h.in "${PROJECT_BINARY_DIR}/gen/tools/version.h" @ONLY)

add_library(XFFS2Play MODULE
    xpmain.cpp
    xpmain.h
    gui/mainframe.cpp
    gui/mainframe.h
    net/iudpserver.h
    net/lockedqueue.h
    net/proto.h
    net/udpserver.cpp
    net/udpserver.h
    sim/aircraft.cpp
    sim/aircraft.h
    sim/aircraftAI.cpp
    sim/aircraftAI.h
    sim/aircraftCSL.cpp
    sim/aircraftCSL.h
    sim/aircraftstate.cpp
    sim/aircraftstate.h
    sim/simcontroller.cpp
    sim/simcontroller.h
    sim/useraircraft.cpp
    sim/useraircraft.h
    tools/color.cpp
    tools/color.h
    tools/datetime.cpp
    tools/datetime.h
    tools/hexa.h
    tools/log.cpp
    tools/log.h
    tools/ramp.cpp
    tools/ramp.h
    tools/simple_file_parser.cpp
    tools/simple_file_parser.h
    tools/utils3d.h
    "${PROJECT_BINARY_DIR}/gen/tools/version.h")

target_include_directories(XFFS2Play PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}/gen"
    ${XPlaneSDK_INCLUDE_DIRS})

target_link_libraries(XFFS2Play PRIVATE
    xffs2play_proto
    XPMP2
    asio::asio
    OpenGL::GL
    Threads::Threads)

target_compile_definitions(XFFS2Play PRIVATE
    ASIO_NO_DEPRECATED
    $<$<BOOL:${XFFS_DEBUG}>:XFFS_DEBUG=1>)

# X-Plane loads <plugin>/64/{win,lin,mac}.xpl
set_target_properties(XFFS2Play PROPERTIES PREFIX "" SUFFIX ".xpl")

if(WIN32)
    set_target_properties(XFFS2Play PROPERTIES OUTPUT_NAME "win")
    target_compile_definitions(XFFS2Play PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601)
    target_link_libraries(XFFS2Play PRIVATE XPlaneSDK::XPLM ws2_32 wsock32 iphlpapi)
elseif(APPLE)
    set_target_properties(XFFS2Play PROPERTIES OUTPUT_NAME "mac")
    target_link_libraries(XFFS2Play PRIVATE XPlaneSDK::XPLM "-framework CoreFoundation")
else()
    set_target_properties(XFFS2Play PROPERTIES OUTPUT_NAME "lin")
    # XPLM symbols are resolved by X-Plane at load time; bundle the C++ runtime for portability
    target_link_options(XFFS2Play PRIVATE -static-libstdc++ -static-libgcc)
endif()

if(MSVC)
    target_compile_options(XFFS2Play PRIVATE /W4 /utf-8 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(XFFS2Play PRIVATE -Wall -Wextra)
endif()

################################################################################
# Installation: <prefix>/xffs2play/{64/<os>.xpl, Resources/...}
################################################################################

install(TARGETS XFFS2Play
        LIBRARY DESTINATION xffs2play/64
        RUNTIME DESTINATION xffs2play/64)
install(DIRECTORY Resources/ DESTINATION xffs2play/Resources)
