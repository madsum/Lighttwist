# - Find BMC
#
# find the bmc library and includes
#
# This module defines
#  BMC_INCLUDE_DIR, where to find bimulticat.h, etc.
#  BMC_LIBRARIES, the libraries needed to use BMC.
#  BMC_FOUND, If false, do not try to use BMC.
# also defined, but not for general use are
#  BMC_LIBRARY, where to find the BMC library.

FIND_PATH(BMC_INCLUDE_DIR bimulticast.h PATHS /usr/local/include /usr/include
    PATH_SUFFIXES bmc)

SET(BMC_NAMES ${BMC_NAMES} bmc m ssl)
FIND_LIBRARY(BMC_LIBRARY
  NAMES ${BMC_NAMES}
  PATHS /usr/lib /usr/local/lib
  )


IF (BMC_LIBRARY AND BMC_INCLUDE_DIR)
    SET(BMC_LIBRARIES ${BMC_LIBRARY})
    SET(BMC_FOUND "YES")
ELSE (BMC_LIBRARY AND BMC_INCLUDE_DIR)
  SET(BMC_FOUND "NO")
ENDIF (BMC_LIBRARY AND BMC_INCLUDE_DIR)

IF (BMC_FOUND)
   IF (NOT BMC_FIND_QUIETLY)
      MESSAGE(STATUS "Found BMC: ${BMC_LIBRARIES}")
   ENDIF (NOT BMC_FIND_QUIETLY)
ELSE (BMC_FOUND)
   IF (BMC_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find BMC library")
   ENDIF (BMC_FIND_REQUIRED)
ENDIF (BMC_FOUND)

# Deprecated declarations.
#SET (NATIVE_BMC_INCLUDE_PATH ${BMC_INCLUDE_DIR} )
#GET_FILENAME_COMPONENT (NATIVE_BMC_LIB_PATH ${BMC_LIBRARY} PATH)

MARK_AS_ADVANCED(
  BMC_LIBRARY
  BMC_INCLUDE_DIR
  )

