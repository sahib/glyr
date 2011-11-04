#include "../../core.h"
#include "../../glyr.h"
#include "../../stringlib.h"
#include "../../register_plugins.h"

static gboolean path_go_up(char * song_dir_path);
static GList * find_in_musictree(const gchar * song_file_path, const gchar * regex, gint up_to,GlyrQuery * query);
static void foreach_file(const gchar * song_dir_path, const GRegex * cRegex, GlyrQuery * query, GList ** retv_list);

//---------------------------------------------------

gboolean is_file(const gchar * song_path_dir, const gchar * elem_name)
{
    gchar * absolute_path = g_strdup_printf("%s%c%s",song_path_dir,G_DIR_SEPARATOR,elem_name);
    gboolean retv = g_file_test(absolute_path,G_FILE_TEST_IS_REGULAR);
    g_free(absolute_path);
    return retv;  
}

//---------------------------------------------------

static gchar * get_file_contents(const gchar * filename, gsize * len)
{
    gchar * content = NULL;
    GError * file_error = NULL;
    if(filename != NULL)
    {
        if(g_file_get_contents(filename,&content,len,&file_error) == FALSE)
        {
            glyr_message(-1,NULL,"Error while reading file '%s': %s\n",filename,file_error->message);
            g_error_free(file_error);
        }
    }
    return content;
}

//---------------------------------------------------

static GLYR_DATA_TYPE get_data_type_from_query(GlyrQuery * query)
{
    GLYR_DATA_TYPE type = GLYR_TYPE_NOIDEA;
    for(GList * elem = r_getFList(); elem != NULL; elem = elem->next)
    {
        MetaDataFetcher * fetcher = elem->data;
        if(fetcher != NULL && fetcher->type == query->type)
        {
            type = fetcher->default_data_type;
            break;
        }
    }
    return type;
}

//---------------------------------------------------

static void file_to_cache(GList ** list, GlyrQuery * query, gchar * data, gsize size)
{
    if(list != NULL && data != NULL)
    {
        GlyrMemCache * item = DL_init();
        glyr_cache_set_data(item,data,size);
        item->type = get_data_type_from_query(query);
        item->dsrc = g_strdup("musictree");
        item->prov = g_strdup("musictree");

        if(TYPE_IS_IMAGE(query->type))
        {
            item->img_format = g_strdup("jpeg");
            item->is_image = TRUE;
        }
        else
        {
            item->is_image = FALSE;
        }

        *list = g_list_prepend(*list, item);
    }
}

//---------------------------------------------------

static void foreach_file(const gchar * song_dir_path, const GRegex * cRegex, GlyrQuery * query, GList ** retv_list)
{
    if(song_dir_path != NULL)
    {
        GError * dir_error = NULL;
        GDir * song_dir = g_dir_open(song_dir_path,0,&dir_error);
        if(song_dir != NULL)
        {
            const gchar * entry = NULL;
            while((entry = g_dir_read_name(song_dir)) != NULL)
            {
                if(regex_match_compiled(entry,cRegex) == TRUE)
                {

                    gsize size = 0;
                    gchar * absolute_path = g_strdup_printf("%s%c%s",song_dir_path,G_DIR_SEPARATOR,entry);

                    if(absolute_path != NULL)
                    {
                        /* Read file */
                        gchar * retv = get_file_contents(absolute_path,&size);
                        g_free(absolute_path);

                        /* Add file to result list if not null */
                        file_to_cache(retv_list,query,retv,size);

                        /* Not requested more? */
                        if(g_list_length(*retv_list) >= (gsize)query->number)
                            break;
                    }
                }
            }
            g_dir_close(song_dir);
        }
        else
        {
            glyr_message(-1,NULL,"Opening %s failed: %s\n",song_dir_path,dir_error->message);
            g_error_free(dir_error);
        }
    }
}

//---------------------------------------------------

static gboolean path_go_up(char * song_dir_path)
{
    if(song_dir_path != NULL)
    {
        gchar * sep = strrchr(song_dir_path,G_DIR_SEPARATOR);
        if(sep != NULL && sep != song_dir_path)
        {
            *sep = 0;
            return TRUE;
        }

        if(sep != NULL && sep == song_dir_path)
        {
            sep[1] = 0;
            return TRUE;
        }
    }

    return FALSE;
}

//---------------------------------------------------

static GList * find_in_musictree(const gchar * song_file_path, const gchar * regex, gint up_to,GlyrQuery * query)
{
    GList * retv_list = NULL;
    gchar * song_dir_path = g_path_get_dirname(song_file_path);

    if(song_dir_path != NULL)
    {
        /* Compile the regex just once */
        GRegex * compiled_regex = g_regex_new(regex,G_REGEX_CASELESS, 0, NULL);
        if(compiled_regex != NULL)
        {
            for(gint i = 0; i < up_to; ++i)
            {
                /* Check every single file */
                foreach_file(song_dir_path,compiled_regex, query, &retv_list);

                /* Cannot go up anymore */
                if(path_go_up(song_dir_path) == FALSE)
                    break;
            }

            /* Forgot the regex */
            g_regex_unref(compiled_regex);
        }
        g_free(song_dir_path);
    }
    return retv_list;
}

//---------------------------------------------------

/* Dirty Hack :-( */
static gchar * configure_regex(const gchar * regex, GlyrQuery * subs)
{
    gchar * correct_regex = NULL;
    if(regex && subs)
    {
        GlyrQuery temp;
        temp.artist = g_regex_escape_string(subs->artist,-1);
        temp.album  = g_regex_escape_string(subs->album, -1);
        temp.title  = g_regex_escape_string(subs->title ,-1);
        correct_regex = prepare_url(regex,&temp,FALSE);

        g_free(temp.artist);
        g_free(temp.album );
        g_free(temp.title );
    }   
    return correct_regex;
}

//---------------------------------------------------
//---------------------------------------------------

/* PROVIDER STUFF */

static const gchar * musictree_provider_url(GlyrQuery * query)
{
    return OFFLINE_PROVIDER; 
}

//---------------------------------------------------

static GList * musictree_provider_parse(cb_object * capo)
{
    GList * retv_list = NULL;

    gint recurse_depth;
    const gchar * search_regex = NULL;

    /* The actual regexes are shamelessly ripped of GMPC */
    switch(capo->s->type)
    {
        case GLYR_GET_COVERART:
            search_regex = "^(folder|cover|.*${album}.*)\\.(jpg|png|jpeg|gif)";
            recurse_depth = 2;
            break;
        case GLYR_GET_ARTIST_PHOTOS:
            search_regex = "^(${artist}|artist)\\.(jpg|png|jpeg|gif)$";
            recurse_depth = 3;
            break;
        case GLYR_GET_ALBUM_REVIEW:
            search_regex = "^(${album})\\.(info|txt)$";
            recurse_depth = 2;
            break;
        case GLYR_GET_ARTISTBIO:
            search_regex = "^BIOGRAPHY(\\.txt)?$";
            recurse_depth = 2;
            break;
        default: 
            search_regex = NULL;
            recurse_depth = 0;
            break;
    }

    if(search_regex != NULL && capo->s->musictree_path != NULL)
    {
        gchar * correct_regex = configure_regex(search_regex,capo->s);
        retv_list = find_in_musictree(capo->s->musictree_path,correct_regex,recurse_depth,capo->s);

        g_free(correct_regex);
    }
    return retv_list;
}

//---------------------------------------------------

MetaDataSource musictree_provider_src =
{
    .name = "musictree",
    .key  = 't',
    .parser    = musictree_provider_parse,
    .get_url   = musictree_provider_url,
    .type      = GLYR_GET_ANY,
    .quality   = G_MAXINT,
    .speed     = G_MAXINT,
    .endmarker = NULL,
    .free_url  = false
};
