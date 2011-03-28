%module glyr

%{
#include "../lib/glyr.h"
%}

//------------------------------------//
//------------ DECLARATION -----------//
//------------------------------------//

enum GLYR_GET_TYPE
{
    GET_COVER,
    GET_LYRIC,
    GET_PHOTO,
    GET_AINFO,
    GET_SIMILIAR,
    GET_REVIEW,
    GET_TRACKLIST,
    GET_TAGS,
    GET_RELATIONS,
    GET_ALBUMLIST,
    GET_UNSURE
};

// This is not a duplicate of GLYR_GET_TYPE
// (more to follow)
enum GLYR_DATA_TYPE
{
    TYPE_NOIDEA,
    TYPE_LYRICS,
    TYPE_REVIEW,
    TYPE_PHOTOS,
    TYPE_COVER,
    TYPE_COVER_PRI,
    TYPE_COVER_SEC,
    TYPE_AINFO,
    TYPE_SIMILIAR,
    TYPE_ALBUMLIST,
    TYPE_TAGS,
    TYPE_TAG_ARTIST,
    TYPE_TAG_ALBUM,
    TYPE_TAG_TITLE,
    TYPE_RELATION,
    TYPE_TRACK
};

/* Group IDs */
enum GLYR_GROUPS
{
    /* Groups are build by (a | b)*/
    GRP_NONE = 0 << 0, /* None    */
    GRP_SAFE = 1 << 0, /* Safe    */
    GRP_USFE = 1 << 1, /* Unsafe  */
    GRP_SPCL = 1 << 2, /* Special */
    GRP_FAST = 1 << 3, /* Fast    */
    GRP_SLOW = 1 << 4, /* Slow    */
    GRP_ALL  = 1 << 5  /* All!    */
};

enum GLYR_ERROR
{
    GLYRE_OK,           // everything is fine
    GLYRE_BAD_OPTION,   // you passed a bad option to Gly_setopt()
    GLYRE_BAD_VALUE,    // Invalid value in va_list
    GLYRE_EMPTY_STRUCT, // you passed an empty struct to Gly_setopt()
    GLYRE_NO_PROVIDER,  // setttings->provider == NULL
    GLYRE_UNKNOWN_GET,  // settings->type is not valid
    GLYRE_STOP_BY_CB    // Callback returned stop signal.
};


// ----------
//  DEFINES 
// ----------

// Colors. For whatever.
#define C_B "\x1b[34;01m" // blue
#define C_M "\x1b[35;01m" // magenta
#define C_C "\x1b[36;01m" // Cyan
#define C_R "\x1b[31;01m" // Red
#define C_G "\x1b[32;01m" // Green
#define C_Y "\x1b[33;01m" // Yellow
#define C_  "\x1b[0m"     // Reset

// Change this if you really need more
#define DEFAULT_TIMEOUT  20L
#define DEFAULT_REDIRECTS 1L
#define DEFAULT_PARALLEL 4L
#define DEFAULT_CMINSIZE 125
#define DEFAULT_CMAXSIZE -1
#define DEFAULT_VERBOSITY 0
#define DEFAULT_NUMBER 1
#define DEFAULT_PLUGMAX -1
#define DEFAULT_LANG "en"
#define DEFAULT_DOWNLOAD true
#define DEFAULT_GROUPEDL true
#define DEFAULT_FROM_ARGUMENT_DELIM ";"
#define DEFAULT_DUPLCHECK true
#define DEFAULT_FUZZYNESS 4
#define DEFAULT_FORMATS "jpg;jpeg;png"
#define DEFAULT_CALL_DIRECT_USE false
#define DEFAULT_CALL_DIRECT_PROVIDER NULL

#define API_KEY_DISCOGS "adff651383"
#define API_KEY_AMAZON  "AKIAJ6NEA642OU3FM24Q"
#define API_KEY_LASTFM  "7199021d9c8fbae507bf77d0a88533d7"
#define API_KEY_FLICKR  "b5af0c3230fb478d53b20835223d57a4"

/* Group names */
#define GRPN_NONE "none"
#define GRPN_SAFE "safe"
#define GRPN_USFE "unsafe"
#define GRPN_SPCL "special"
#define GRPN_FAST "fast"
#define GRPN_SLOW "slow"
#define GRPN_ALL  "all"

typedef int (*DL_callback)(GlyMemCache * dl, struct GlyQuery * s);

//------------------------------------//
//------------- DATATYPES ------------//
//------------------------------------//

// +++++++++++++++++++++++++++++++++++++++++++++++++++++

// Internal buffer model
typedef struct GlyMemCache
{
    char  *data;    // data buffer
    size_t size;    // Size of data
    char  *dsrc;    // Source of data
    int   type;     // type of metadata
    int   error;    // error code - internal use only
    int   duration; // Duration of a song. Only for tracklist getter.
    bool  is_image; // Wether it is an image or not
} GlyMemCache;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++

// list of GlyMemCaches
typedef struct GlyCacheList
{
    GlyMemCache ** list;
%immutable;
    size_t size;
    int usersig;
%mutable;
} GlyCacheList;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct GlyQuery
{
    // get
    int type;

    // of
    char * artist;
    char * album;
    char * title;

    // number
    int number;
    int plugmax;

    // cover
    struct
    {
        int min_size;
        int max_size;
    } cover;

    // from (as void because I didnt manage to prototype GlyPlugin... *?*)
    void * providers;

    // invoke() control
    long parallel;
    long timeout;
    long redirects;

    // verbosity
    int verbosity;
    // use colored output?
    bool color_output;

    // return only urls without downloading?
    // this converts glyr to a sort of search engines
    bool download;

    // Download group for group,
    // or all in parallel? (faster, but less accurate)
    bool groupedDL;

    // Check for bad data?
    bool duplcheck;

    // Treshold for Levenshtein
    size_t fuzzyness;

%immutable;
    // max ten slots, 5 are used now
    const char * info[10];

    // allowed formats for images
    const char * formats;

    // language settings (for amazon / google / last.fm)
    const char * lang;

    // count of dlitems, starting from 0
    int itemctr;

    struct callback
    {
        int  (* download)(GlyMemCache * dl, struct GlyQuery * s);
        void  * user_pointer;
    } callback;

    struct call_direct
    {
        bool use;
        const char * provider;
        const char * url;
    } call_direct;

%mutable;

} GlyQuery;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct GlyPlugin
{
%immutable;
    const char * name;  // Full name
    const char * key;   // A one-letter ID
    const char * color; // Colored name
    int use;            // Use this source?

    struct
    {
        // Passed to the corresponding cb_object and is called...perhaps
        GlyCacheList * (* parser_callback) (struct cb_object *);
        const char *   (* url_callback)    (GlyQuery  *);
        const char *  endmarker; // Stop download if containing this string
        bool free_url; // pass result of url_callback to free()?
    } plug;

    unsigned char gid;
%mutable;
} GlyPlugin;


//------------------------------------//
//------------- FUNCTIONS ------------//
//------------------------------------//

%rename(get) Gly_get;
GlyCacheList * Gly_get(GlyQuery * settings, int * error);

%rename(initQuery) Gly_init_query;
void Gly_init_query(GlyQuery * glyrs);

%rename(destroyQuery) Gly_destroy_query;
void Gly_destroy_query(GlyQuery * sets);

%rename(freeList) Gly_free_list;
void Gly_free_list(GlyCacheList * lst);

%rename(list_at) Gly_clist_at;
GlyMemCache * Gly_clist_at(GlyCacheList * clist, int iter);

%rename(new_cache) Gly_new_cache;
GlyMemCache * Gly_new_cache(void);

%rename(freeCache) Gly_free_cache;
void Gly_free_cache(GlyMemCache * c);


//-- setopts --//
int GlyOpt_dlcallback(GlyQuery * settings, DL_callback dl_cb, void * userp);
int GlyOpt_type(GlyQuery * s, enum GLYR_GET_TYPE type);
int GlyOpt_artist(GlyQuery * s, char * artist);
int GlyOpt_album(GlyQuery * s,  char * album);
int GlyOpt_title(GlyQuery * s,  char * title);
int GlyOpt_cmaxsize(GlyQuery * s, int size);
int GlyOpt_cminsize(GlyQuery * s, int size);
int GlyOpt_parallel(GlyQuery * s, unsigned long val);
int GlyOpt_timeout(GlyQuery * s, unsigned long val);
int GlyOpt_redirects(GlyQuery * s, unsigned long val);
int GlyOpt_lang(GlyQuery * s, char * langcode);
int GlyOpt_number(GlyQuery * s, unsigned int num);
int GlyOpt_verbosity(GlyQuery * s, unsigned int level);
int GlyOpt_from(GlyQuery * s, const char * from);
int GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife);
int GlyOpt_plugmax(GlyQuery * s, int plugmax);
int GlyOpt_download(GlyQuery * s, bool download);
int GlyOpt_groupedDL(GlyQuery * s, bool groupedDL);
int GlyOpt_formats(GlyQuery * s, const char * formats);
int GlyOpt_fuzzyness(GlyQuery * s, int fuzz);
int GlyOpt_call_direct_use(GlyQuery * s, bool use);
int GlyOpt_call_direct_provider(GlyQuery * s, const char * provider);
int GlyOpt_call_direct_url(GlyQuery * s, const char * URL);

%rename(version) Gly_version;
const char * Gly_version(void);

%rename(ProviderByID) Gly_get_provider_by_id; 
GlyPlugin * Gly_get_provider_by_id(int ID);

%rename(download) Gly_download;
GlyMemCache * Gly_download(const char * url, GlyQuery * s);

%rename(BinaryWrite) Gly_write_binary_file;
int Gly_write_binary_file(const char * path, GlyMemCache * data, const char * save_dir, const char * type, GlyQuery *s);

%rename(groupname_by_id) Gly_groupname_by_id;
const char * Gly_groupname_by_id(int ID);

%rename(strerror) Gly_strerror;
const char * Gly_strerror(enum GLYR_ERROR ID);

//---------------//
//--- EXTENDS ---//
//---------------//

%extend GlyCacheList
{
        GlyCacheList()
        {
                GlyCacheList * v = malloc(sizeof(GlyCacheList));
                v->list = NULL;
                v->size = 0;
                return v;
        }
        ~GlyCacheList()
        {
                Gly_free_list($self);
        }
}

%extend GlyQuery
{
        GlyQuery()
        {
                GlyQuery my_query;
                Gly_init_query(&my_query);
                GlyQuery * copy = malloc(sizeof(GlyQuery));
                memcpy(copy,&my_query,sizeof(GlyQuery));
                return copy;
        }
        ~GlyQuery()
        {
                Gly_destroy_query($self);
                if($self != NULL)
                    free($self);
        }
}

%extend GlyMemCache
{
        GlyMemCache()
        {
                return Gly_new_cache();
        }
        ~GlyMemCache()
        {
                Gly_free_cache($self);
        }
}
