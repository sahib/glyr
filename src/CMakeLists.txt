ADD_EXECUTABLE(glyrc main.c)
ADD_EXECUTABLE(example example.c)
ADD_EXECUTABLE(threads threads.c)
ADD_EXECUTABLE(simple simple.c)

#install
install(TARGETS glyrc RUNTIME DESTINATION bin)

# link with
TARGET_LINK_LIBRARIES(glyrc glyr) 
TARGET_LINK_LIBRARIES(example  glyr) 
TARGET_LINK_LIBRARIES(threads  glyr) 
TARGET_LINK_LIBRARIES(simple   glyr) 

