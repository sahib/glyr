ADD_EXECUTABLE(glyrc glyrc/glyrc.c glyrc/autohelp.c glyrc/update.c glyrc/escape.c glyrc/colorprint.c)
TARGET_LINK_LIBRARIES(glyrc glyr)

ADD_EXECUTABLE(example examples/example.c)
TARGET_LINK_LIBRARIES(example glyr) 

ADD_EXECUTABLE(threads examples/threads.c)
TARGET_LINK_LIBRARIES(threads glyr) 

ADD_EXECUTABLE(simple  examples/simple.c)
TARGET_LINK_LIBRARIES(simple glyr) 

ADD_EXECUTABLE(dbcache examples/dbcache.c)
TARGET_LINK_LIBRARIES(dbcache glyr) 

ADD_EXECUTABLE(rating  examples/rating.c)
TARGET_LINK_LIBRARIES(rating glyr) 

ADD_EXECUTABLE(getinfo examples/getinfo.c)
TARGET_LINK_LIBRARIES(getinfo glyr) 

ADD_EXECUTABLE(exit_signal examples/exit_signal.c)
TARGET_LINK_LIBRARIES(exit_signal glyr) 

ADD_EXECUTABLE(async_queue examples/async_queue.c)
TARGET_LINK_LIBRARIES(async_queue glyr)

ADD_EXECUTABLE(ping_url utils/ping_url.c)
TARGET_LINK_LIBRARIES(ping_url glyr) 

ADD_EXECUTABLE(musictree examples/musictree.c)
TARGET_LINK_LIBRARIES(musictree glyr) 

ADD_EXECUTABLE(guess_lang utils/guess_lang.c)
TARGET_LINK_LIBRARIES(guess_lang glyr) 

ADD_EXECUTABLE(clean_db utils/clean_db.c)
TARGET_LINK_LIBRARIES(clean_db glyr) 

#install
INSTALL(TARGETS glyrc RUNTIME DESTINATION ${INSTALL_BIN_DIR})
