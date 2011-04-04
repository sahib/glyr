# configure
set( GLYR_VERSION_MAJOR "0")
set( GLYR_VERSION_MINOR "4")
set( GLYR_VERSION_NAME  "Larcenous Locust (dev)")
set( GLYR_COLORED_OUTPUT true)

configure_file (
  "${DIR_ROOT}/config.h.in"
  "${DIR_ROOT}/config.h"
)

ADD_DEFINITIONS( -ggdb3 -Wall )
ADD_LIBRARY(glyr SHARED	${LIB_SOURCE_LOCATIONS})

if(WIN32)
  TARGET_LINK_LIBRARIES(glyr curl ws2_32) 
else()
  TARGET_LINK_LIBRARIES(glyr curl) 
endif()

# Install header
INSTALL(FILES glyr.h DESTINATION include)

# Install library 
INSTALL(TARGETS glyr LIBRARY DESTINATION lib)
