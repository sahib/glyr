# Configuration for Version /versionnam
configure_file (
  "${DIR_TOP}/lib/config.h.in"
  "${DIR_TOP}/lib/config.h"
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
INSTALL(TARGETS glyr LIBRARY DESTINATION ${INSTALL_LIB_DIR})
