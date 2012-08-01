PKG_CHECK_MODULES(LIBCHECK_PKG check REQUIRED)
INCLUDE_DIRECTORIES(${LIBCHECK_PKG_INCLUDE_DIRS})

ADD_LIBRARY(test_common STATIC test_common.c)
TARGET_LINK_LIBRARIES(test_common ${LIBCHECK_PKG_LIBRARIES} glyr)

ADD_EXECUTABLE(check_api check_api.c)
ADD_EXECUTABLE(check_opt check_opt.c)
ADD_EXECUTABLE(check_dbc check_dbc.c)
TARGET_LINK_LIBRARIES(check_api glyr test_common)
TARGET_LINK_LIBRARIES(check_opt glyr test_common)
TARGET_LINK_LIBRARIES(check_dbc glyr test_common)
