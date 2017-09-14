add_executable (ltprecrop ltprecrop.cpp PrecropMain.cpp PrecropThread.cpp)
target_link_libraries (ltprecrop ${IMGU_LIBS} ${LT_LIBS})

install(TARGETS ltprecrop RUNTIME DESTINATION ${LT_BIN})

