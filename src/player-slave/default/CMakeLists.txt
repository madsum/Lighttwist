add_executable (ltplayer ltplayer.cpp ReceiveThread.cpp)
target_link_libraries (ltplayer player_layers)
install(TARGETS ltplayer RUNTIME DESTINATION ${LT_BIN})

