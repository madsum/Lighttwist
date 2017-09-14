# - Find LUA
# Find the native LUA includes and library
# This module defines
#  LUA_INCLUDE_DIR, where to find jpeglib.h, etc.
#  LUA_LIBRARIES, the libraries needed to use LUA.
#  LUA_FOUND, If false, do not try to use LUA.
# also defined, but not for general use are
#  LUA_LIBRARY, where to find the LUA library.

FIND_PATH(LUA_INCLUDE_DIR lua.h PATHS /usr/local/include /usr/include PATH_SUFFIXES lua5.1)

SET(LUA_NAMES ${LUA_NAMES} lua5.1 lua.5.1.4)
FIND_LIBRARY(LUA_LIBRARY
  NAMES ${LUA_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (LUA_LIBRARY AND LUA_INCLUDE_DIR)
    SET(LUA_LIBRARIES ${LUA_LIBRARY})
    SET(LUA_FOUND "YES")
ELSE (LUA_LIBRARY AND LUA_INCLUDE_DIR)
  SET(LUA_FOUND "NO")
ENDIF (LUA_LIBRARY AND LUA_INCLUDE_DIR)


IF (LUA_FOUND)
   IF (NOT LUA_FIND_QUIETLY)
      MESSAGE(STATUS "Found LUA: ${LUA_LIBRARIES}")
   ENDIF (NOT LUA_FIND_QUIETLY)
ELSE (LUA_FOUND)
   IF (LUA_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find LUA library")
   ENDIF (LUA_FIND_REQUIRED)
ENDIF (LUA_FOUND)

# Deprecated declarations.
SET (NATIVE_LUA_INCLUDE_PATH ${LUA_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_LUA_LIB_PATH ${LUA_LIBRARY} PATH)

MARK_AS_ADVANCED(
  LUA_LIBRARY
  LUA_INCLUDE_DIR
  )

