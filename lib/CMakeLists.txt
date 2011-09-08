# Configuration for Version /versionnam
configure_file (
  "${DIR_TOP}/lib/config.h.in"
  "${DIR_TOP}/lib/config.h"
)

# Link libglyr as shared library
ADD_LIBRARY(glyr SHARED	${LIB_SOURCE_LOCATIONS})

# Win32 needs that socket library for libcurl
if(WIN32)
  TARGET_LINK_LIBRARIES(glyr curl ${GLIBPKG_LIBRARIES} ws2_32) 
else(WIN32) 
  TARGET_LINK_LIBRARIES(glyr curl ${GLIBPKG_LIBRARIES}) 
endif(WIN32)

# Install Files
INSTALL(FILES glyr.h  DESTINATION ${INSTALL_INC_DIR}/glyr)
INSTALL(FILES types.h DESTINATION ${INSTALL_INC_DIR}/glyr)
INSTALL(TARGETS glyr LIBRARY DESTINATION ${INSTALL_LIB_DIR})
