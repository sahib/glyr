ADD_EXECUTABLE(glyrc main.c)
ADD_EXECUTABLE(example example.c)

#install
install(TARGETS glyrc RUNTIME DESTINATION  bin)

# link with
TARGET_LINK_LIBRARIES(glyrc glyr) 
TARGET_LINK_LIBRARIES(example  glyr) 

