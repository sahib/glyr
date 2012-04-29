from glypy import *

# C Examples need to do this before using the lib:
#   glyr_init()
#   atexit(glyr_cleanup)
# this is already done by importing the module already

# Instance a new query
# this needs to be initialized seperatelely to default values
q = GlyrQuery()
glyr_query_init(q)

# Configure the Query
glyr_opt_artist(q,'Akrea')
glyr_opt_album(q,'Lebenslinie')
# title is not needed for coverart
glyr_opt_title(q,'Trugbild')
# You could do GLYR_GET_LYRICS too.
glyr_opt_type(q,GLYR_GET_COVERART)
glyr_opt_verbosity(q,4)

# Instance a Cache and tell
# the query to use it.
db = glyr_db_init('/tmp')
glyr_opt_lookup_db(q,db)

# Now send the query on it's long way
# First run will catch it from the web, 2nd from the cache
# (2nd and 3rd parametre does not work yet)
c = glyr_get(q,None,None)

# Bad thing (TM): 
# You cannot acces c.data directly for image data,
# since python somehow seems to interprete the data as utf-8
# So the only useful thing for images is to write them to a file
glyr_cache_print(c)
print('The items was gathered from the cache:',c.cached)
print('Originally from {prov} ({url}) and is {size} bytes large'.format(
    prov = c.prov,
    url  = c.dsrc,
    size = c.size
    ))

# The checksum is interally raw data too, so we need to convert it
# glyr has it's own converting routines here
print('Finally its checksum: ', glyr_md5sum_to_string(c.md5sum))

# Write the data somewhere on the HD
# Textual items can be used directly.
glyr_cache_write(c,'/tmp/cover.' + c.img_format)
