# - Find SWIG
# This module defines
#  SWIG_INCLUDE_DIR, where to find jpeglib.h, etc.
#  SWIG_FOUND, If false, do not try to use SWIG.
# also defined, but not for general use are

FIND_PATH(SWIG_INCLUDE_DIR swig PATHS /usr/bin)

IF (SWIG_INCLUDE_DIR)
    SET(SWIG_FOUND "YES")
ELSE (SWIG_INCLUDE_DIR)
  SET(SWIG_FOUND "NO")
ENDIF (SWIG_INCLUDE_DIR)


IF (SWIG_FOUND)
   IF (NOT SWIG_FIND_QUIETLY)
      MESSAGE(STATUS "Found SWIG")
   ENDIF (NOT SWIG_FIND_QUIETLY)
ELSE (SWIG_FOUND)
   IF (SWIG_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SWIG")
   ENDIF (SWIG_FIND_REQUIRED)
ENDIF (SWIG_FOUND)

MARK_AS_ADVANCED(
  SWIG_INCLUDE_DIR
)

