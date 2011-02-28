set (DIR_ROOT      ${PROJECT_BINARY_DIR}/${SUBDIR_LIB}  )
set (DIR_COVER     ${DIR_ROOT}/cover                    )
set (DIR_LYRICS    ${DIR_ROOT}/lyrics                   )
set (DIR_PHOTOS    ${DIR_ROOT}/photos                   )
set (DIR_BOOKS     ${DIR_ROOT}/books                    )
set (DIR_AINFO     ${DIR_ROOT}/ainfo                    )
set (DIR_SIMILIAR  ${DIR_ROOT}/similiar                 )
set (DIR_REVIEW    ${DIR_ROOT}/review                   )

# configure
set( GLYR_VERSION_MAJOR "0")
set( GLYR_VERSION_MINOR "2")
set( GLYR_VERSION_NAME  "Kleptomaniacal Magpie")
set( GLYR_COLORED_OUTPUT true)

configure_file (
  "${DIR_ROOT}/config.h.in"
  "${DIR_ROOT}/config.h"
)

ADD_LIBRARY(glyr SHARED 
	"${DIR_ROOT}/glyr.c"
	"${DIR_ROOT}/core.c"
	"${DIR_ROOT}/stringop.c"
	"${DIR_ROOT}/ainfo.c"
	"${DIR_ROOT}/books.c"
	"${DIR_ROOT}/cover.c"
	"${DIR_ROOT}/similiar.c"
	"${DIR_ROOT}/lyrics.c"
	"${DIR_ROOT}/photos.c"
	"${DIR_ROOT}/review.c"
	"${DIR_AINFO}/lastfm.c"
	"${DIR_AINFO}/allmusic_com.c"
	"${DIR_SIMILIAR}/lastfm.c"
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
    	"${DIR_BOOKS}/dnb.c"
    	"${DIR_BOOKS}/kit.c"
)
 
ADD_DEFINITIONS( -ggdb3 ) 

install(TARGETS glyr LIBRARY DESTINATION lib)

IF(CMAKE_CROSSCOMPILING)
 TARGET_LINK_LIBRARIES(glyr curl ws2_32 )
 REMOVE_DEFINITIONS( -fPIC -rdynamic ) 
ELSE(CMAKE_CROSSCOMPILING)
 TARGET_LINK_LIBRARIES(glyr curl) 
ENDIF(CMAKE_CROSSCOMPILING)
