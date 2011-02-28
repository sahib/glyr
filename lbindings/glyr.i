%module glyr

%{
#include "../lib/glyr.h"
%}

//------------------------------------//
//------------ DECLARATION -----------//
//------------------------------------//

enum GLYR_ERROR
{
    GLYRE_OK,           
    GLYRE_BAD_OPTION,   
    GLYRE_BAD_VALUE,    
    GLYRE_EMPTY_STRUCT, 
    GLYRE_NO_PROVIDER,  
    GLYRE_UNKNOWN_GET   
};

// get types enum
enum GLYR_GET_TYPES
{
    GET_COVER,
    GET_LYRIC,
    GET_PHOTO,
    GET_AINFO,
    GET_SIMILIAR,
    GET_REVIEW,
    GET_BOOKS,
    GET_UNSURE
};

// Colors. For whatever.
#define C_B "\x1b[34;01m" // blue
#define C_M "\x1b[35;01m" // magenta
#define C_C "\x1b[36;01m" // Cyan
#define C_R "\x1b[31;01m" // Red
#define C_G "\x1b[32;01m" // Green
#define C_Y "\x1b[33;01m" // Yellow
#define C_  "\x1b[0m"     // Reset

#define DEFAULT_TIMEOUT  20L
#define DEFAULT_REDIRECTS 1L
#define DEFAULT_PARALLEL 4L
#define DEFAULT_CMINSIZE 125
#define DEFAULT_CMAXSIZE -1
#define DEFAULT_VERBOSITY 2
#define DEFAULT_NUMBER 1
#define DEFAULT_PLUGMAX 7
#define DEFAULT_LANG GLYRL_UK
#define DEFAULT_DOWNLOAD true
#define DEFAULT_FROM_ARGUMENT_DELIM ";"


//------------------------------------//
//------------- DATATYPES ------------//
//------------------------------------//

typedef struct GlyMemCache
{
    char  *data;
    size_t size; 
    char  *dsrc;  
%immutable;
    int   error;   
%mutable;

} GlyMemCache;

typedef struct GlyCacheList
{
    GlyMemCache ** list;

%immutable;
    size_t size;
%mutable;

} GlyCacheList;

typedef struct GlyQuery
{
    int type;

    char * artist;
    char * album;
    char * title;

    int number;
    int plugmax;

    struct
    {
        int min_size;
        int max_size;
    } cover;

    void * providers;

    long parallel;
    long timeout;
    long redirects;

    int verbosity;
    bool color_output;

    bool download;
    char * lang;

%immutable;
    int itemctr;
%mutable;

%immutable;
    struct callback {
        void (* download)(GlyMemCache * dl, struct GlyQuery * s);
        void  * user_pointer;
    } callback;

    const char * info[PTR_SPACE];
%mutable;

} GlyQuery;

typedef void (*callback_t) (void *user_data, const char *other_data);
void anvoke(callback_t callback, void *user_data, const char *other_data);

typedef struct GlyPlugin
{
%immutable;
    const char * name;  
    const char * key;  
    const char * color; 
    int use;            

    struct
    {
        GlyCacheList * (* parser_callback) (struct cb_object *);
        const char *   (* url_callback)    (GlyQuery  *);
        bool free_url; // pass result of url_callback to free()?
    } plug;
%mutable;
} GlyPlugin;

%{
void
wrap_callback(void *user_data, const char *other_data)
{
  VALUE proc = (VALUE)user_data;
  rb_funcall(proc, rb_intern("call"), 1, rb_str_new2(other_data));
}
%}

%typemap(in) (callback_t callback, void *user_data)
{
  $1 = wrap_callback;
  $2 = (void *)$input;
}

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

%rename(freeCache) Gly_free_cache;
void Gly_free_cache(GlyMemCache * c);

%rename(pushList) Gly_push_to_list;
void Gly_push_to_list(GlyCacheList * l, GlyMemCache * c);

//-- setopts --//
int GlyOpt_dlcallback(GlyQuery * settings, void (*dl_cb)(GlyMemCache *, GlyQuery *), void * userp);
int GlyOpt_type(GlyQuery * s, int type);
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
int GlyOpt_infoat(GlyQuery * s, int at, const char * value);
int GlyOpt_from(GlyQuery * s, const char * from);
int GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife);
int GlyOpt_plugmax(GlyQuery * s, int plugmax);
int GlyOpt_download(GlyQuery * s, bool download);

%rename(list_at) Gly_clist_at;
GlyMemCache * Gly_clist_at(GlyCacheList * clist, int iter);

%rename(new_cache) Gly_new_cache;
GlyMemCache * Gly_new_cache(void);

%rename(version) Gly_version;
const char * Gly_version(void);

%rename(ProviderByID) Gly_get_provider_by_id; 
GlyPlugin * Gly_get_provider_by_id(int ID);

%rename(BinaryWrite) Gly_write_binary_file;
int Gly_write_binary_file(const char * path, GlyMemCache * data, const char * save_dir, const char * type, GlyQuery *s);

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
