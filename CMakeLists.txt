cmake_minimum_required(VERSION 2.6)
PROJECT(glyr C)

include(FindPkgConfig)

SET(GLYR_DEBUG true)
SET(CSTANDARD "-std=gnu99")
SET(CDEBUG "-ggdb3")
SET(CWARN "-Wall -Wstrict-prototypes")
SET(COPT "-Os -march=native -fomit-frame-pointer")

if("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
	SET(CFLAGS "${CDEBUG} ${CWARN} ${CSTANDARD}")
else()
	SET(CFLAGS "${CDEBUG} ${CWARN} ${CSTANDARD}")
	SET(CMAKE_EXE_LINKER_FLAGS "-s")
endif()

SET(CMAKE_C_FLAGS ${CFLAGS})

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
	${PROJECT_BINARY_DIR}/bin
	CACHE PATH
	"Single Directory for all Libraries"
	)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
	${PROJECT_BINARY_DIR}/bin
	CACHE PATH
	"Single Directory for all Executables."
	)

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
	${PROJECT_BINARY_DIR}/bin
	CACHE PATH
	"Single Directory for all static libraries."
	)

# set directories
SET(SUBDIR_LIB lib)
SET(SUBDIR_SRC src)
SET(SUBDIR_SWIG swig)

# configure
set(GLYR_VERSION_MAJOR "0")
set(GLYR_VERSION_MINOR "8.5")
set(GLYR_VERSION_NAME  "Horny Hornet [beta]")

configure_file (
  "libglyr.pc.in"
  "libglyr.pc"
)
INSTALL(FILES libglyr.pc DESTINATION lib/pkgconfig) 

# Find deps
FIND_PACKAGE(CURL REQUIRED)
PKG_CHECK_MODULES(GLIBPKG glib-2.0>=2.10 gthread-2.0 REQUIRED)
INCLUDE_DIRECTORIES(${GLIBPKG_INCLUDE_DIRS})

set (DIR_TOP ${PROJECT_BINARY_DIR})
set (DIR_ROOT ${PROJECT_BINARY_DIR}/${SUBDIR_LIB} )
set (DIR_INTERN ${DIR_ROOT}/intern )
set (DIR_COVER ${DIR_INTERN}/cover )
set (DIR_LYRICS ${DIR_INTERN}/lyrics )
set (DIR_PHOTOS ${DIR_INTERN}/photos )
set (DIR_AINFO ${DIR_INTERN}/ainfo )
set (DIR_SIMILIAR ${DIR_INTERN}/similiar_artist )
set (DIR_SIMILISO ${DIR_INTERN}/similiar_song )
set (DIR_REVIEW ${DIR_INTERN}/review )
set (DIR_TRACKLIST ${DIR_INTERN}/tracklist )
set (DIR_ALBUMLIST ${DIR_INTERN}/albumlist )
set (DIR_TAGS ${DIR_INTERN}/tags )
set (DIR_RELATIONS ${DIR_INTERN}/relations )
set (DIR_GUITARTABS ${DIR_INTERN}/guitartabs )

SET(LIB_SOURCE_LOCATIONS
	# core
	"${DIR_ROOT}/glyr.c"
	"${DIR_ROOT}/core.c"
	"${DIR_ROOT}/md5.c"
	"${DIR_ROOT}/register_plugins.c"
	"${DIR_ROOT}/stringlib.c"
	"${DIR_ROOT}/blacklist.c"
	# fetcher
	"${DIR_INTERN}/generic.c"
	"${DIR_INTERN}/common.c"
	"${DIR_INTERN}/ainfo.c"
	"${DIR_INTERN}/cover.c"
	"${DIR_INTERN}/similiar_artist.c"
	"${DIR_INTERN}/similiar_song.c"
	"${DIR_INTERN}/lyrics.c"
	"${DIR_INTERN}/photos.c"
	"${DIR_INTERN}/review.c"
	"${DIR_INTERN}/tracklist.c"
	"${DIR_INTERN}/tags.c"
	"${DIR_INTERN}/relations.c"
	"${DIR_INTERN}/albumlist.c"
	"${DIR_INTERN}/guitartabs.c"
	# extensions
	"${DIR_AINFO}/lastfm.c"
	"${DIR_AINFO}/allmusic_com.c"
	"${DIR_AINFO}/lyricsreg.c"
	"${DIR_SIMILIAR}/lastfm.c"
	"${DIR_SIMILISO}/lastfm.c"
	"${DIR_TRACKLIST}/musicbrainz.c"
	"${DIR_REVIEW}/allmusic_com.c"
	"${DIR_REVIEW}/amazon.c"
	"${DIR_REVIEW}/metallum.c"
	"${DIR_ALBUMLIST}/musicbrainz.c"
	"${DIR_RELATIONS}/musicbrainz.c"
	"${DIR_TAGS}/musicbrainz.c"
	"${DIR_COVER}/lastfm.c"
	"${DIR_COVER}/google.c"
	"${DIR_COVER}/coverhunt.c"
	"${DIR_COVER}/lyricswiki.c"
	"${DIR_COVER}/albumart.c"
	"${DIR_COVER}/allmusic_com.c"
	"${DIR_COVER}/discogs.c"
	"${DIR_COVER}/amazon.c"
	"${DIR_COVER}/rhapsody.c"
	"${DIR_LYRICS}/lyrdb.c"
	"${DIR_LYRICS}/metallum.c"
	"${DIR_LYRICS}/magistrix.c"
	"${DIR_LYRICS}/lyrix_at.c"
	"${DIR_LYRICS}/lyricsvip.c"
	"${DIR_LYRICS}/metrolyrics.c"
	"${DIR_LYRICS}/lyricswiki.c"
	"${DIR_LYRICS}/lyricstime.c"
	"${DIR_LYRICS}/lyricsreg.c"
	"${DIR_LYRICS}/lipwalk.c"
	"${DIR_LYRICS}/elyrics.c"
	"${DIR_PHOTOS}/flickr.c"
	"${DIR_PHOTOS}/lastfm.c"
	"${DIR_PHOTOS}/google.c"
	"${DIR_PHOTOS}/discogs.c"
	"${DIR_PHOTOS}/singerpictures.c"
	"${DIR_PHOTOS}/rhapsody.c"
	"${DIR_GUITARTABS}/guitaretab.c"
	"${DIR_GUITARTABS}/chordie_com.c"
#       "${DIR_LYRICS}/songlyrics.c"
#       "${DIR_LYRICS}/directlyrics.c"
#       "${DIR_LYRICS}/darklyrics.c"
)

# Pretty high warnlevel
ADD_DEFINITIONS(-Wall -W -Wno-unused-parameter -Wno-strict-prototypes)

# go on with subdirs
ADD_SUBDIRECTORY( ${SUBDIR_LIB} )
ADD_SUBDIRECTORY( ${SUBDIR_SRC} )

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
    )

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Ruby bindings
# You gonna need swig && ruby 1.9.1 (+headers)
IF(SWIG_RUBY)
	SET(INTERFACE_FILE "swig/glyros.i")
	FIND_PACKAGE(SWIG REQUIRED)
	INCLUDE(${SWIG_USE_FILE})

	FIND_PACKAGE(Ruby)
	INCLUDE_DIRECTORIES(${RUBY_INCLUDE_PATH})

	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

	SET(CMAKE_SWIG_FLAGS "")

	SWIG_ADD_MODULE(glyros ruby ${INTERFACE_FILE} ${LIB_SOURCE_LOCATIONS})
	SWIG_LINK_LIBRARIES(glyros ${RUBY_LIBRARY} ${CURL_LIBRARY} ${GLIBPKG_LIBRARIES})

	ADD_DEFINITIONS(-Wno-unused-but-set-variable -Wno-unused-variable)

	ADD_CUSTOM_COMMAND (
		TARGET glyros
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rename bin/libglyros.so swig/ruby/glyros.so
		)
ENDIF(SWIG_RUBY)


