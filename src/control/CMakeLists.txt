if(JSW_FOUND)
  add_executable (ltjoystick ltjoystick.c)
  target_link_libraries (ltjoystick ${IMGU_LIBS} ${LT_LIBS} ${BMC_LIBS} ${JS_LIBS})

  install(TARGETS ltjoystick RUNTIME DESTINATION ${LT_BIN})

  install(FILES joystick_calib DESTINATION ${LT_SHARE})
endif()




