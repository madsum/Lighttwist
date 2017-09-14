# - Find AVCODEC
# Find the native AVCODEC includes and library
# This module defines
#  AVCODEC_INCLUDE_DIR, where to find jpeglib.h, etc.
#  AVCODEC_LIBRARIES, the libraries needed to use AVCODEC.
#  AVCODEC_FOUND, If false, do not try to use AVCODEC.
# also defined, but not for general use are
#  AVCODEC_LIBRARY, where to find the AVCODEC library.

FIND_PATH(AVCODEC_INCLUDE_DIR avcodec.h PATHS /usr/local/include /usr/include PATH_SUFFIXES ffmpeg libavcodec)

SET(AVCODEC_NAMES ${AVCODEC_NAMES} avcodec)
FIND_LIBRARY(AVCODEC_LIBRARY
  NAMES ${AVCODEC_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (AVCODEC_LIBRARY AND AVCODEC_INCLUDE_DIR)
    SET(AVCODEC_LIBRARIES ${AVCODEC_LIBRARY})
    SET(AVCODEC_FOUND "YES")
ELSE (AVCODEC_LIBRARY AND AVCODEC_INCLUDE_DIR)
  SET(AVCODEC_FOUND "NO")
ENDIF (AVCODEC_LIBRARY AND AVCODEC_INCLUDE_DIR)


IF (AVCODEC_FOUND)
   IF (NOT AVCODEC_FIND_QUIETLY)
      MESSAGE(STATUS "Found AVCODEC: ${AVCODEC_LIBRARIES}")
   ENDIF (NOT AVCODEC_FIND_QUIETLY)
ELSE (AVCODEC_FOUND)
   IF (AVCODEC_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find AVCODEC library")
   ENDIF (AVCODEC_FIND_REQUIRED)
ENDIF (AVCODEC_FOUND)

# Deprecated declarations.
SET (NATIVE_AVCODEC_INCLUDE_PATH ${AVCODEC_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_AVCODEC_LIB_PATH ${AVCODEC_LIBRARY} PATH)

MARK_AS_ADVANCED(
  AVCODEC_LIBRARY
  AVCODEC_INCLUDE_DIR
  )

