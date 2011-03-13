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
SET( SUBDIR_BIND bind)

# find libcurl dependency
FIND_PACKAGE(CURL REQUIRED)

set (DIR_ROOT      ${PROJECT_BINARY_DIR}/${SUBDIR_LIB}  )
set (DIR_COVER     ${DIR_ROOT}/cover                    )
set (DIR_LYRICS    ${DIR_ROOT}/lyrics                   )
set (DIR_PHOTOS    ${DIR_ROOT}/photos                   )
set (DIR_AINFO     ${DIR_ROOT}/ainfo                    )
set (DIR_SIMILIAR  ${DIR_ROOT}/similiar                 )
set (DIR_REVIEW    ${DIR_ROOT}/review                   )
set (DIR_TRACKLIST ${DIR_ROOT}/tracklist                )

SET(LIB_SOURCE_LOCATIONS 
	"${DIR_ROOT}/glyr.c"
	"${DIR_ROOT}/core.c"
	"${DIR_ROOT}/stringop.c"
	"${DIR_ROOT}/ainfo.c"
	"${DIR_ROOT}/cover.c"
	"${DIR_ROOT}/similiar.c"
	"${DIR_ROOT}/lyrics.c"
	"${DIR_ROOT}/photos.c"
	"${DIR_ROOT}/review.c"
	"${DIR_ROOT}/tracklist.c"
	"${DIR_AINFO}/lastfm.c"
	"${DIR_AINFO}/allmusic_com.c"
	"${DIR_SIMILIAR}/lastfm.c"
	"${DIR_TRACKLIST}/musicbrainz.c"
	"${DIR_REVIEW}/allmusic_com.c"
	"${DIR_COVER}/last_fm.c"
	"${DIR_COVER}/google.c"
	"${DIR_COVER}/coverhunt.c"
	"${DIR_COVER}/lyricswiki.c"
	"${DIR_COVER}/albumart.c"
	"${DIR_COVER}/allmusic_com.c"
	"${DIR_COVER}/discogs.c"
	"${DIR_COVER}/amazon.c"
        "${DIR_LYRICS}/lyrdb.c"
    	"${DIR_LYRICS}/magistrix.c"
    	"${DIR_LYRICS}/lyrix_at.c"
	"${DIR_LYRICS}/lyricsvip.c"
    	"${DIR_LYRICS}/directlyrics.c"
    	"${DIR_LYRICS}/songlyrics.c"
    	"${DIR_LYRICS}/darklyrics.c"
    	"${DIR_LYRICS}/metrolyrics.c"
    	"${DIR_LYRICS}/lyricswiki.c"
    	"${DIR_PHOTOS}/flickr.c"
    	"${DIR_PHOTOS}/lastfm.c"
)

ADD_DEFINITIONS( -Wall -Winline -W -Wwrite-strings -Werror -Wno-unused-parameter )

# go on with subdirs
ADD_SUBDIRECTORY( ${SUBDIR_LIB}  )
ADD_SUBDIRECTORY( ${SUBDIR_SRC}  )
#ADD_SUBDIRECTORY( ${SUBDIR_BIND} )
