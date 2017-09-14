CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
#cmake_policy( SET CMP0002 OLD)
#cmake_policy( SET CMP0003 NEW)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules/")

project(LIGHTTWIST)
set(LIGHTTWIST_VERSION_MAJOR "1")
set(LIGHTTWIST_VERSION_MINOR "0")
set(LIGHTTWIST_VERSION_PATCH "0")
set(LIGHTTWIST_VERSION "${LIGHTTWIST_VERSION_MAJOR}.${LIGHTTWIST_VERSION_MINOR}")

#installation directories relative to prefix path CMAKE_INSTALL_PREFIX:PATH
set(LT_BIN bin)
set(LT_LIB lib)
set(LT_SHARE share/lighttwist)

set(MATRIX_LIBS matrixmath)
set(IMGU_LIBS imgu)

OPTION(LT_OSG_FRAMEWORKS "Use OSG Frameworks" ON)

find_package(AVFORMAT)
if(AVFORMAT_FOUND)
  add_definitions(-DHAVE_LIBAVFORMAT)
  include_directories(${AVFORMAT_INCLUDE_DIR})
  #add_definitions(${AVFORMAT_DEFINITIONS})
  link_directories(${AVFORMAT_LINK_DIRECTORIES})
else()
  message(FATAL_ERROR "Cannot build without AVFORMAT")
  #message("Could not find libavformat")
endif()



find_package(AVCODEC)
if(AVCODEC_FOUND)
  add_definitions(-DHAVE_LIBAVCODEC)
  include_directories(${AVCODEC_INCLUDE_DIR})
  #add_definitions(${AVCODEC_DEFINITIONS})
  link_directories(${AVCODEC_LINK_DIRECTORIES})
else()
  message(FATAL_ERROR "Cannot build without AVCODEC")
  #message("Could not find libavcodec")
endif()

find_package(AVUTIL)
if(AVUTIL_FOUND)
  add_definitions(-DHAVE_LIBAVUTIL)
  include_directories(${AVUTIL_INCLUDE_DIR})
  #add_definitions(${AVUTIL_DEFINITIONS})
  link_directories(${AVUTIL_LINK_DIRECTORIES})
else()
  message(FATAL_ERROR "Cannot build without AVUTIL")
  #message("Could not find libavutil")
endif()

find_package(SWSCALE)
if(SWSCALE_FOUND)
  add_definitions(-DHAVE_LIBSWSCALE)
  include_directories(${SWSCALE_INCLUDE_DIR})
  link_directories(${SWSCALE_LINK_DIRECTORIES})
else()
  message(FATAL_ERROR "Cannot build without SWSCALE")
endif()

find_package(JSW)
if(JSW_FOUND)
  add_definitions(-DHAVE_LIBJSW)
  include_directories(${JSW_INCLUDE_DIR})
  link_directories(${JSW_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without JSW")
  #message("Could not find JSW")
endif()

find_package(LUA)
if(LUA_FOUND)
  add_definitions(-DHAVE_LIBLUA)
  include_directories(${LUA_INCLUDE_DIR})
  #add_definitions(${LUA_DEFINITIONS})
  link_directories(${LUA_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without LUA")
  message("Could not find liblua5.1 (package lua5.1 liblua5.1-0-dev)")
endif()


find_package(PTHREAD)
if(PTHREAD_FOUND)
  add_definitions(-DHAVE_PTHREAD)
  #set(IMGU_LIBS ${IMGU_LIBS} pthread)
  include_directories(${PTHREAD_INCLUDE_DIR})
  link_directories(${PTHREAD_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without pthread")
  message("Could not find pthread")
endif()


find_package(BMC)
if(BMC_FOUND)
   add_definitions(-DHAVE_BMC)
   include_directories(${BMC_INCLUDE_DIR})
   link_directories(${BMC_LINK_DIRECTORIES})
   set(BMC_LIBS bmc m ssl)
else()
   message(FATAL_ERROR "Cannot build without BMC")
endif()

find_package(SWIG)
if(SWIG_FOUND)

else()
  #message(FATAL_ERROR "Cannot build without SWIG")
  message("Could not find SWIG")
endif()

find_package(OSG)
if(OSG_FOUND)
  add_definitions(-DHAVE_OSG)
  include_directories(${OSG_INCLUDE_DIR})
  link_directories(${OSG_LINK_DIRECTORIES})
  if(APPLE)
    set(OSG_LIBS "-framework osg -framework osgDB -framework osgViewer -framework OpenThreads")
#  else()
#    set(OSG_LIBS osg osgDB osgViewer)
  endif()
else()
  message(FATAL_ERROR "Cannot build without OSG")
  #message("Could not find OSG")
endif()

#-----------------------------------------------------------------------------
### lib and path setup
#-----------------------------------------------------------------------------
include_directories(/usr/include /usr/local/include ${LIGHTTWIST_SOURCE_DIR}/src ${LIGHTTWIST_SOURCE_DIR}/src/player-slave/lib-layers/)
set(IMGU_LIBS ${IMGU_LIBS} ${MATRIX_LIBS})

set(JS_LIBS jsw)
set(LT_LIBS ltutils)
set(AV_LIBS ${AVUTIL_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES})
set(LUA_LIBS ${LUA_LIBRARIES})

if(APPLE)
  set(AV_LIBS ${AV_LIBS} z bz2)
endif()

if(APPLE)
  set(GUI_LIBS fltk fltk_images "-framework Carbon")
else()
  if(WIN32)
    set(GUI_LIBS fltk fltk_images opengl32 glut32win glu32 gdi32 ole32 ws2_32 uuid)
    add_definitions(-D_STDCALL_SUPPORTED -D_M_IX86)
  else()
    set(GUI_LIBS fltk fltk_images Xft)
  endif()
endif()

add_subdirectory (src)

#-----------------------------------------------------------------------------
### for packaging
#-----------------------------------------------------------------------------
set(CPACK_PACKAGE_VERSION ${LIGHTTWIST_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${LIGHTTWIST_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIGHTTWIST_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIGHTTWIST_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lighttwist")
set(CPACK_PACKAGE_VENDOR "University of Montreal")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "lighttwist-${LIGHTTWIST_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "${LIGHTTWIST_SOURCE_DIR}/test/")
include(CPack)

#######################
#this is for uninstallation
#requires file CMakeModules/cmake_uninstall.cmake.in
#######################
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P
"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")




