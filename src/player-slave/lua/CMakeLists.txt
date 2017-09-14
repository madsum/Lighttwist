if (SWIG_FOUND)
  ADD_CUSTOM_COMMAND(
        DEPENDS luawrapfunctions.i
        COMMAND swig -c++ -lua luawrapfunctions.i
        OUTPUT luawrapfunctions_wrap.cxx
  )

  if (LUA_FOUND)
    add_executable (ltplayerlua ltplayerlua.cpp luawrapfunctions.cpp luawrapfunctions_wrap.cxx ImguPluginTexture.cpp ltimgupluginlayer.cpp)
    target_link_libraries (ltplayerlua player_layers ${LUA_LIBS})
    install(TARGETS ltplayerlua RUNTIME DESTINATION ${LT_BIN})
  endif()
endif()

install(FILES commands.lua DESTINATION ${LT_SHARE})

