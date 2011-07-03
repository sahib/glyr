cmake_minimum_required(VERSION 2.6)
PROJECT( glyr C )

SET(CSTANDARD "-std=gnu99")
SET(CDEBUG "-ggdb3")
SET(CWARN "-Wall -Wstrict-prototypes")
SET(COPT "-Os -march=native -fomit-frame-pointer")

if("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
	SET(CFLAGS "${CDEBUG} ${CWARN} ${CSTANDARD}")
else()
	SET(CFLAGS "${CDEBUG} ${CWARN} ${CSTANDARD}")
	#SET(CFLAGS "${COPT} ${CWARN} ${CSTANDARD}")
	SET(CMAKE_EXE_LINKER_FLAGS "-s")
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
SET( SUBDIR_SWIG swig)

# find libcurl dependency
FIND_PACKAGE(CURL REQUIRED)

set (DIR_ROOT ${PROJECT_BINARY_DIR}/${SUBDIR_LIB} )
set (DIR_COVER ${DIR_ROOT}/cover )
set (DIR_LYRICS ${DIR_ROOT}/lyrics )
set (DIR_PHOTOS ${DIR_ROOT}/photos )
set (DIR_AINFO ${DIR_ROOT}/ainfo )
set (DIR_SIMILIAR ${DIR_ROOT}/similiar_artist )
set (DIR_SIMILISO ${DIR_ROOT}/similiar_song )
set (DIR_REVIEW ${DIR_ROOT}/review )
set (DIR_TRACKLIST ${DIR_ROOT}/tracklist )
set (DIR_ALBUMLIST ${DIR_ROOT}/albumlist )
set (DIR_TAGS ${DIR_ROOT}/tags )
set (DIR_RELATIONS ${DIR_ROOT}/relations )

SET(LIB_SOURCE_LOCATIONS
	# core
	"${DIR_ROOT}/glyr.c"
	"${DIR_ROOT}/core.c"
	"${DIR_ROOT}/stringlib.c"
	"${DIR_ROOT}/ainfo.c"
	"${DIR_ROOT}/cover.c"
	"${DIR_ROOT}/similiar_artist.c"
	"${DIR_ROOT}/similiar_song.c"
	"${DIR_ROOT}/lyrics.c"
	"${DIR_ROOT}/photos.c"
	"${DIR_ROOT}/review.c"
	"${DIR_ROOT}/tracklist.c"
	"${DIR_ROOT}/tags.c"
	"${DIR_ROOT}/relations.c"
	"${DIR_ROOT}/albumlist.c"
	"${DIR_ROOT}/translate.c"
	"${DIR_ROOT}/md5.c"
	# extensions
	"${DIR_AINFO}/lastfm.c"
	"${DIR_AINFO}/allmusic_com.c"
	"${DIR_AINFO}/lyricsreg.c"
	"${DIR_SIMILIAR}/lastfm.c"
	"${DIR_SIMILISO}/lastfm.c"
	"${DIR_TRACKLIST}/musicbrainz.c"
	"${DIR_REVIEW}/allmusic_com.c"
	"${DIR_REVIEW}/amazon.c"
	"${DIR_ALBUMLIST}/musicbrainz.c"
	"${DIR_RELATIONS}/musicbrainz.c"
	"${DIR_TAGS}/musicbrainz.c"
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
	"${DIR_LYRICS}/lyricstime.c"
	"${DIR_LYRICS}/lyricsreg.c"
	"${DIR_PHOTOS}/flickr.c"
	"${DIR_PHOTOS}/lastfm.c"
	"${DIR_PHOTOS}/google.c"
	)

# Pretty high warnlevel
ADD_DEFINITIONS(-Wall -W -Wno-unused-parameter -Wno-strict-prototypes)

# go on with subdirs
ADD_SUBDIRECTORY( ${SUBDIR_LIB} )
ADD_SUBDIRECTORY( ${SUBDIR_SRC} )

IF(SWIG_RUBY)
	SET(INTERFACE_FILE "swig/glyros.i")
	FIND_PACKAGE(SWIG REQUIRED)
	INCLUDE(${SWIG_USE_FILE})

	FIND_PACKAGE(Ruby)
	INCLUDE_DIRECTORIES(${RUBY_INCLUDE_PATH})

	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

	SET(CMAKE_SWIG_FLAGS "")

	SWIG_ADD_MODULE(glyros ruby ${INTERFACE_FILE} ${LIB_SOURCE_LOCATIONS})
	SWIG_LINK_LIBRARIES(glyros ${RUBY_LIBRARY} ${CURL_LIBRARY})

	ADD_DEFINITIONS(-Wno-unused-but-set-variable)

	ADD_CUSTOM_COMMAND (
		TARGET glyros
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory bin/ruby
		)
	ADD_CUSTOM_COMMAND (
		TARGET glyros
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rename bin/libglyros.so bin/ruby/glyros.so
		)
ENDIF(SWIG_RUBY)


