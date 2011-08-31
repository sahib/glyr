ADD_EXECUTABLE(glyrc glyrc/glyrc.c glyrc/autohelp.c)
ADD_EXECUTABLE(example examples/example.c)
ADD_EXECUTABLE(threads examples/threads.c)
ADD_EXECUTABLE(simple  examples/simple.c)
ADD_EXECUTABLE(ping_url utils/ping_url.c)

#install
install(TARGETS glyrc RUNTIME DESTINATION bin)

# link with
TARGET_LINK_LIBRARIES(glyrc    glyr )
TARGET_LINK_LIBRARIES(example  glyr ) 
TARGET_LINK_LIBRARIES(threads  glyr ) 
TARGET_LINK_LIBRARIES(simple   glyr ) 
TARGET_LINK_LIBRARIES(ping_url glyr ) 
