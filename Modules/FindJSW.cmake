# - Find JSW
# Find the native JSW includes and library
# This module defines
#  JSW_INCLUDE_DIR, where to find jpeglib.h, etc.
#  JSW_LIBRARIES, the libraries needed to use JSW.
#  JSW_FOUND, If false, do not try to use JSW.
# also defined, but not for general use are
#  JSW_LIBRARY, where to find the JSW library.

FIND_PATH(JSW_INCLUDE_DIR jsw.h PATHS /usr/local/include /usr/include)

SET(JSW_NAMES ${JSW_NAMES} jsw)
FIND_LIBRARY(JSW_LIBRARY
  NAMES ${JSW_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (JSW_LIBRARY AND JSW_INCLUDE_DIR)
    SET(JSW_LIBRARIES ${JSW_LIBRARY})
    SET(JSW_FOUND "YES")
ELSE (JSW_LIBRARY AND JSW_INCLUDE_DIR)
  SET(JSW_FOUND "NO")
ENDIF (JSW_LIBRARY AND JSW_INCLUDE_DIR)


IF (JSW_FOUND)
   IF (NOT JSW_FIND_QUIETLY)
      MESSAGE(STATUS "Found JSW: ${JSW_LIBRARIES}")
   ENDIF (NOT JSW_FIND_QUIETLY)
ELSE (JSW_FOUND)
   IF (JSW_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find JSW library")
   ENDIF (JSW_FIND_REQUIRED)
ENDIF (JSW_FOUND)

# Deprecated declarations.
SET (NATIVE_JSW_INCLUDE_PATH ${JSW_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_JSW_LIB_PATH ${JSW_LIBRARY} PATH)

MARK_AS_ADVANCED(
  JSW_LIBRARY
  JSW_INCLUDE_DIR
  )

