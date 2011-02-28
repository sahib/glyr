ADD_EXECUTABLE(glyrc main.c)


#install
install(TARGETS glyrc RUNTIME DESTINATION  bin)

# link with
TARGET_LINK_LIBRARIES(glyrc glyr curl) 
