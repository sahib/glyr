# configure
set( GLYR_VERSION_MAJOR "0")
set( GLYR_VERSION_MINOR "4")
set( GLYR_VERSION_NAME  "Larcenous Locust")
set( GLYR_COLORED_OUTPUT true)

configure_file (
  "${DIR_ROOT}/config.h.in"
  "${DIR_ROOT}/config.h"
)

ADD_DEFINITIONS( -ggdb3 -Wall )
ADD_LIBRARY(glyr SHARED	${LIB_SOURCE_LOCATIONS})
TARGET_LINK_LIBRARIES(glyr curl) 
INSTALL(TARGETS glyr LIBRARY DESTINATION lib)
