CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Write in C, write in Ceeee... :-)
PROJECT(glyr C)

# Include pkg-config
INCLUDE(FindPkgConfig)

# ------------------------------------------------
# You can configure the next few params on the cmd:
# -DLIB_SUFFIX="64" for example
# ------------------------------------------------
IF(NOT DEFINED INSTALL_BIN_DIR)
 SET(INSTALL_BIN_DIR bin)
ELSE()
  MESSAGE("-- Install binaries to: \"${INSTALL_BIN_DIR}\"")
ENDIF()

IF(NOT DEFINED INSTALL_LIB_DIR)
  SET(INSTALL_LIB_DIR lib${LIB_SUFFIX})
ELSE()
  MESSAGE("-- Install library to: \"${INSTALL_LIB_DIR}\"")
ENDIF()

IF(NOT DEFINED INSTALL_INC_DIR)
  SET(INSTALL_INC_DIR include)
ELSE()
  MESSAGE("-- Install headers to: \"${INSTALL_INC_DIR}\"")
ENDIF()

IF(DEFINED LIB_SUFFIX)
  MESSAGE("-- Library suffix is: \"${LIB_SUFFIX}\"")
ENDIF()
# ------------------------------------------------

# ------------------------------------------------
# Versioning 
# ------------------------------------------------
SET(GLYR_VERSION_MAJOR "0")
SET(GLYR_VERSION_MINOR "9")
SET(GLYR_VERSION_MICRO "1")
SET(GLYR_VERSION_NAME  "Catholic Cat")
# ------------------------------------------------


# ------------------------------------------------
# CFlags and warnlevel / build config 
# ------------------------------------------------

# Tell config.h.in
IF(CMAKE_BUILD_TYPE STREQUAL "debug")
    SET(GLYR_DEBUG TRUE)
ELSE()
    SET(GLYR_DEBUG FALSE)
ENDIF()

SET(COMMON_FLAGS "-Wall -Wextra -Wstrict-prototypes -std=c99 -W -Wno-unused-parameter -Wno-strict-prototypes")
SET(CMAKE_C_FLAGS_RELEASE "${COMMON_FLAGS} ${CMAKE_C_FLAGS} -Os -march=native -s")
SET(CMAKE_C_FLAGS_DEBUG   "${COMMON_FLAGS} ${CMAKE_C_FLAGS} -ggdb3")
SET(CMAKE_C_FLAGS "${COMMON_FLAGS} ${CMAKE_C_FLAGS}")
# ------------------------------------------------

# ------------------------------------------------
# Configuration & pkg-config
# ------------------------------------------------
CONFIGURE_FILE(
  "libglyr.pc.in"
  "libglyr.pc"
)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libglyr.pc DESTINATION ${INSTALL_LIB_DIR}/pkgconfig) 

# ----------------------
# Find deps
# ----------------------
FIND_PACKAGE(CURL REQUIRED)
PKG_CHECK_MODULES(GLIBPKG glib-2.0>=2.10 gthread-2.0 REQUIRED)
PKG_CHECK_MODULES(SQLITE3 sqlite3 REQUIRED)
INCLUDE_DIRECTORIES(${GLIBPKG_INCLUDE_DIRS})

# --------------------------
# set directories
# --------------------------

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables.")

SET(SUBDIR_LIB lib)
SET(SUBDIR_SRC src)
SET(SUBDIR_SWIG swig)
SET(DIR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR_LIB}) 
SET(DIR_INTERN ${DIR_ROOT}/intern )
SET(DIR_COVER ${DIR_INTERN}/cover )
SET(DIR_LYRICS ${DIR_INTERN}/lyrics )
SET(DIR_PHOTOS ${DIR_INTERN}/photos )
SET(DIR_AINFO ${DIR_INTERN}/ainfo )
SET(DIR_SIMILAR ${DIR_INTERN}/similar_artist )
SET(DIR_SIMILSO ${DIR_INTERN}/similar_song )
SET(DIR_REVIEW ${DIR_INTERN}/review )
SET(DIR_TRACKLIST ${DIR_INTERN}/tracklist )
SET(DIR_ALBUMLIST ${DIR_INTERN}/albumlist )
SET(DIR_TAGS ${DIR_INTERN}/tags )
SET(DIR_RELATIONS ${DIR_INTERN}/relations )
SET(DIR_GUITARTABS ${DIR_INTERN}/guitartabs )
SET(DIR_BACKDROPS ${DIR_INTERN}/backdrops )

# ------------------------------------------------
# Set source locations
# ------------------------------------------------
SET(LIB_SOURCE_LOCATIONS
	# core
	"${DIR_ROOT}/glyr.c"
	"${DIR_ROOT}/core.c"
	"${DIR_ROOT}/cache_intern.c"
	"${DIR_ROOT}/cache.c"
	"${DIR_ROOT}/register_plugins.c"
	"${DIR_ROOT}/stringlib.c"
	"${DIR_ROOT}/blacklist.c"
    "${DIR_ROOT}/testing.c"
    # "Builtin" special providers
	"${DIR_INTERN}/cache/db_provider.c"
	"${DIR_INTERN}/musictree/musictree.c"
	# fetcher
	"${DIR_INTERN}/generic.c"
	"${DIR_INTERN}/ainfo.c"
	"${DIR_INTERN}/cover.c"
	"${DIR_INTERN}/similar_artist.c"
	"${DIR_INTERN}/similar_song.c"
	"${DIR_INTERN}/lyrics.c"
	"${DIR_INTERN}/photos.c"
	"${DIR_INTERN}/review.c"
	"${DIR_INTERN}/tracklist.c"
	"${DIR_INTERN}/tags.c"
	"${DIR_INTERN}/relations.c"
	"${DIR_INTERN}/albumlist.c"
	"${DIR_INTERN}/guitartabs.c"
	"${DIR_INTERN}/backdrops.c"
	# extensions
	"${DIR_INTERN}/common/google.c"
	"${DIR_INTERN}/common/amazon.c"
	"${DIR_INTERN}/common/picsearch.c"
	"${DIR_INTERN}/common/musicbrainz.c"
	"${DIR_AINFO}/lastfm.c"
	"${DIR_AINFO}/allmusic_com.c"
	"${DIR_AINFO}/lyricsreg.c"
	"${DIR_SIMILAR}/lastfm.c"
	"${DIR_SIMILSO}/lastfm.c"
	"${DIR_TRACKLIST}/musicbrainz.c"
	"${DIR_REVIEW}/allmusic_com.c"
	"${DIR_REVIEW}/amazon.c"
	"${DIR_REVIEW}/metallum.c"
	"${DIR_ALBUMLIST}/musicbrainz.c"
	"${DIR_RELATIONS}/musicbrainz.c"
	"${DIR_RELATIONS}/generated.c"
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
	"${DIR_COVER}/picsearch.c"
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
	"${DIR_LYRICS}/chartlyrics.c"
	"${DIR_PHOTOS}/flickr.c"
	"${DIR_PHOTOS}/lastfm.c"
	"${DIR_PHOTOS}/google.c"
	"${DIR_PHOTOS}/discogs.c"
	"${DIR_PHOTOS}/singerpictures.c"
	"${DIR_PHOTOS}/rhapsody.c"
	"${DIR_PHOTOS}/picsearch.c"
	"${DIR_GUITARTABS}/guitaretab.c"
	"${DIR_GUITARTABS}/chordie_com.c"
    "${DIR_BACKDROPS}/htbackdrops.c"
# 	Old plugins, removed due to bad quality
#   "${DIR_LYRICS}/songlyrics.c"
#   "${DIR_LYRICS}/directlyrics.c"
#   "${DIR_LYRICS}/darklyrics.c"
)


# ------------------------------------------------
# go on with subdirs
# ------------------------------------------------
ADD_SUBDIRECTORY( ${SUBDIR_LIB} )
ADD_SUBDIRECTORY( ${SUBDIR_SRC} )

IF(DEFINED TEST)
    MESSAGE("-- Building with tests (${TEST})")
    ADD_SUBDIRECTORY( spec/C )
    ADD_CUSTOM_TARGET(test COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/spec/run.rb" DEPENDS check_api)
ENDIF()

# ------------------------------------------------
# uninstall target
# ------------------------------------------------
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
    )

ADD_CUSTOM_TARGET(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
