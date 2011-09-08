ADD_EXECUTABLE(glyrc glyrc/glyrc.c glyrc/autohelp.c)
ADD_EXECUTABLE(example examples/example.c)
ADD_EXECUTABLE(threads examples/threads.c)
ADD_EXECUTABLE(simple  examples/simple.c)
ADD_EXECUTABLE(ping_url utils/ping_url.c)
ADD_EXECUTABLE(guess_lang utils/guess_lang.c)

#install
INSTALL(TARGETS glyrc RUNTIME DESTINATION ${INSTALL_BIN_DIR})

# link with
TARGET_LINK_LIBRARIES(glyrc    glyr)
TARGET_LINK_LIBRARIES(example  glyr) 
TARGET_LINK_LIBRARIES(threads  glyr) 
TARGET_LINK_LIBRARIES(simple   glyr) 
TARGET_LINK_LIBRARIES(ping_url glyr) 
TARGET_LINK_LIBRARIES(guess_lang glyr) 
