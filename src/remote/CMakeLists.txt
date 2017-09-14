add_executable (ltremote ltremote.c)
target_link_libraries (ltremote ${BMC_LIBS})
install(TARGETS ltremote RUNTIME DESTINATION ${LT_BIN})

