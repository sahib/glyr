cmake_minimum_required(VERSION 2.8)

SET(SWIG_USE_FILE glyr.i)

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(Ruby REQUIRED)
INCLUDE_DIRECTORIES(${RUBY_INCLUDE_PATH})

set (DIR_ROOT      ../lib/CMakeFiles/glyr.dir/  )
set (DIR_COVER     ${DIR_ROOT}/cover                    )
set (DIR_LYRICS    ${DIR_ROOT}/lyrics                   )
set (DIR_PHOTOS    ${DIR_ROOT}/photos                   )
set (DIR_BOOKS     ${DIR_ROOT}/books                    )
set (DIR_AINFO     ${DIR_ROOT}/ainfo                    )
set (DIR_SIMILIAR  ${DIR_ROOT}/similiar                 )
set (DIR_REVIEW    ${DIR_ROOT}/review                   )



SET(CMAKE_SWIG_FLAGS "")
#SET_SOURCE_FILES_PROPERTIES(${SWIG_USE_FILE} PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(${SWIG_USE_FILE} PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(glubyr ruby glyr.i)
SWIG_LINK_LIBRARIES(glubyr ${PYTHON_LIBRARIES})
