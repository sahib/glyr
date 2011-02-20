set (DIR_ROOT   .      )
set (DIR_COVER  cover  )
set (DIR_LYRICS lyrics )
set (DIR_PHOTOS photos )
set (DIR_BOOKS  books  )
set (DIR_AINFO  ainfo  )

ADD_LIBRARY( glyr SHARED 
	"${DIR_ROOT}/glyr.c"
	"${DIR_ROOT}/core.c"
	"${DIR_ROOT}/stringop.c"
	"${DIR_ROOT}/ainfo.c"
	"${DIR_ROOT}/books.c"
	"${DIR_ROOT}/cover.c"
	"${DIR_ROOT}/lyrics.c"
	"${DIR_ROOT}/photos.c"
	"${DIR_AINFO}/lastfm.c"
	"${DIR_COVER}/last_fm.c"
	"${DIR_COVER}/google.c"
	"${DIR_COVER}/coverhunt.c"
	"${DIR_COVER}/lyricswiki.c"
	"${DIR_COVER}/albumart.c"
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
