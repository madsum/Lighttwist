if(LUA_FOUND)
  add_library (player_master_lua SHARED extra_pd.c mcnetsend.c luabox.c)
  if (APPLE)
      target_link_libraries (player_master_lua ${BMC_LIBS} ${IMGU_LIBS} ${LT_LIBS} /sw/lib/liblua.5.1.4.dylib)
  else()
      target_link_libraries (player_master_lua ${BMC_LIBS} ${IMGU_LIBS} ${LT_LIBS} lua5.1)
  endif() 
else()
  add_library (player_master_lua SHARED mcnetsend.c)
  target_link_libraries (player_master_lua ${BMC_LIBS} ${IMGU_LIBS} ${LT_LIBS})
endif()
install(TARGETS player_master_lua LIBRARY DESTINATION ${LT_LIB})

SET_TARGET_PROPERTIES(player_master_lua PROPERTIES PREFIX "")
if (APPLE)
  SET_TARGET_PROPERTIES(player_master_lua PROPERTIES LINK_FLAGS "-undefined suppress -flat_namespace")
  SET_TARGET_PROPERTIES(player_master_lua PROPERTIES SUFFIX ".pd_darwin")
else()
  SET_TARGET_PROPERTIES(player_master_lua PROPERTIES SUFFIX ".pd_linux")
endif()

install(FILES midipatch_lune.pd DESTINATION ${LT_SHARE})
install(FILES patch_lune.lua DESTINATION ${LT_SHARE})
install(FILES patch_yukie.lua DESTINATION ${LT_SHARE})
install(FILES Movie.lua DESTINATION ${LT_SHARE})
install(FILES Media.lua DESTINATION ${LT_SHARE})
install(FILES Image.lua DESTINATION ${LT_SHARE})
install(FILES Model.lua DESTINATION ${LT_SHARE})
install(FILES Capture.lua DESTINATION ${LT_SHARE})
install(FILES Crayon.lua DESTINATION ${LT_SHARE})
install(FILES Player.lua DESTINATION ${LT_SHARE})
install(FILES Class.lua DESTINATION ${LT_SHARE})






	



