cmake_minimum_required (VERSION 2.6)

PROJECT(glyr C)

# depends
find_package(CURL REQUIRED)

#versioning
set (glyr_VERSION_MAJOR 0)
set (glyr_VERSION_MINOR 1)

# paths
set (BINARYDIR ./)
set (SOURCEDIR src)
set (SUBDIR_COVER src/cover)
set (SUBDIR_LYRICS src/lyrics)

# tell the compiler how to take care of this 
set (GLYR_LIBCURL "-lcurl")

# Optimizations, if used
#set (GLYR_OPTIMIZ "--march=native -Os -finline-functions -fomit-frame-pointer -s")
set (GLYR_OPTIMIZ "-g -ggdb3")

# Compiler warnings
set (GLYR_WARNSET "-Wall")

#Debug options

# Pass it to cmake
set (CMAKE_CXX_FLAGS "-c -g -pipe ${GLYR_OPTIMIZ} ${GLYR_WARNSET}")
set (CMAKE_EXE_LINKER_FLAGS "-lcurl") 

INCLUDE_DIRECTORIES( include "/usr/include" ) 

# The glyr config header
configure_file (
  "${SOURCEDIR}/glyr_config.h.in"
  "${BINARYDIR}/glyr_config.h"
)

set (CMAKE_DEST_DIR "/usr/bin")
INSTALL(PROGRAMS glyr DESTINATION ${CMAKE_DEST_DIR})

# All source belongs here
ADD_EXECUTABLE(
   "glyr"
   "${SOURCEDIR}/core.c"
   "${SOURCEDIR}/stringop.c" 
   "${SOURCEDIR}/lyrics.c" 
   "${SOURCEDIR}/cover.c" 
   "${SOURCEDIR}/glyr.c" 
   "${SOURCEDIR}/main.c" 
   "${SUBDIR_COVER}/google.c" 
   "${SUBDIR_COVER}/coverhunt.c" 
   "${SUBDIR_COVER}/last_fm.c" 
   "${SUBDIR_COVER}/lyricswiki.c" 
   "${SUBDIR_COVER}/albumart.c" 
   "${SUBDIR_COVER}/discogs.c" 
   "${SUBDIR_COVER}/amazon.c" 
   "${SUBDIR_LYRICS}/lyrdb.c" 
   "${SUBDIR_LYRICS}/magistrix.c" 
   "${SUBDIR_LYRICS}/lyrix_at.c"
   "${SUBDIR_LYRICS}/lyricsvip.c"
   "${SUBDIR_LYRICS}/directlyrics.c"
   "${SUBDIR_LYRICS}/songlyrics.c"
   "${SUBDIR_LYRICS}/darklyrics.c"
   "${SUBDIR_LYRICS}/metrolyrics.c"
   "${SUBDIR_LYRICS}/lyricswiki.c"
)

# Add dependency to target executable
target_link_libraries(glyr ${CURL_LIBRARY})
