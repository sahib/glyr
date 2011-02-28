cmake_minimum_required(VERSION 2.6)
PROJECT( glyr C )

if(CMAKE_COMPILER_IS_GCC)
    SET(CSTANDARD "-std=gnu99")
    SET(CDEBUG "-ggdb3")
    SET(CWARN "-Wall -Wstrict-prototypes")
    SET(COPT "-Os -march=native -fomit-frame-pointer")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
	SET(CFLAGS "${CDEBUG} ${CWARN} ${CSTANDARD}")
else()
	SET(CFLAGS "${CDEBUG} ${CWARN} ${CSTANDARD}")
#	SET(CFLAGS "${COPT} ${CWARN} ${CSTANDARD}")
#        SET(CMAKE_EXE_LINKER_FLAGS "-s") 
endif()

SET(CMAKE_C_FLAGS ${CFLAGS})

# ---------- Setup output Directories -------------------------
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/bin
   CACHE PATH
   "Single Directory for all Libraries"
   )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/bin
   CACHE PATH
   "Single Directory for all Executables."
   )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/bin
   CACHE PATH
   "Single Directory for all static libraries."
   )

# set directories
SET( SUBDIR_LIB lib)
SET( SUBDIR_SRC src)

# find libcurl dependency
FIND_PACKAGE(CURL REQUIRED)

# go on with subdirs
ADD_SUBDIRECTORY( ${SUBDIR_LIB} )
ADD_SUBDIRECTORY( ${SUBDIR_SRC} )
