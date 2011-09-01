# configure
set(GLYR_VERSION_MAJOR "0")
set(GLYR_VERSION_MINOR "8.5")
set(GLYR_VERSION_NAME  "Horny Hornet [beta]")
set(GLYR_COLORED_OUTPUT true)

# Configuration for Version /versionnam
configure_file (
  "${DIR_ROOT}/config.h.in"
  "${DIR_ROOT}/config.h"
)

ADD_DEFINITIONS( -ggdb3 -Wall )
ADD_LIBRARY(glyr SHARED	${LIB_SOURCE_LOCATIONS})

# Win32 needs that socket library for libcurl
if(WIN32)
  TARGET_LINK_LIBRARIES(glyr curl ${GLIBPKG_LIBRARIES} ws2_32) 
else() 
  TARGET_LINK_LIBRARIES(glyr curl ${GLIBPKG_LIBRARIES}) 
endif(WIN32)

# Install headeas
INSTALL(FILES glyr.h DESTINATION include/glyr)
INSTALL(FILES types.h DESTINATION include/glyr)

# Install library 
INSTALL(TARGETS glyr LIBRARY DESTINATION lib)
