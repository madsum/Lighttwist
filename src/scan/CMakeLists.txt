add_executable (ltblend-diffuse ltblend-diffuse.c blend.c)
target_link_libraries (ltblend-diffuse ${IMGU_LIBS} ${LT_LIBS})
add_executable (ltblend-distance ltblend-distance.c blend.c)
target_link_libraries (ltblend-distance ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltblend-diffuse RUNTIME DESTINATION ${LT_BIN})
install(TARGETS ltblend-distance RUNTIME DESTINATION ${LT_BIN})

add_executable (ltcolor ltcolor.c)
target_link_libraries (ltcolor ${IMGU_LIBS} ${LT_LIBS})
add_executable (ltcolorfit ltcolorfit.c)
target_link_libraries (ltcolorfit ${MATRIX_LIBS} ${LT_LIBS})
install(TARGETS ltcolor RUNTIME DESTINATION ${LT_BIN})
install(TARGETS ltcolorfit RUNTIME DESTINATION ${LT_BIN})

add_executable (ltgamma ltgamma.c)
target_link_libraries (ltgamma ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltgamma RUNTIME DESTINATION ${LT_BIN})
add_executable (ltgamma-colorchecker ltgamma-colorchecker.c)
target_link_libraries (ltgamma-colorchecker ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltgamma-colorchecker RUNTIME DESTINATION ${LT_BIN})

add_executable (ltselect ltselect.c select.c)
target_link_libraries (ltselect ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltselect RUNTIME DESTINATION ${LT_BIN})

add_executable (ltsmooth-homography ltsmooth-homography.c)
target_link_libraries (ltsmooth-homography ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltsmooth-homography RUNTIME DESTINATION ${LT_BIN})

add_executable (ltstl ltstl.cpp stl.cpp)
target_link_libraries (ltstl ${LT_LIBS} ${BMC_LIBS} ${OSG_LIBS} ${IMGU_LIBS})
install(TARGETS ltstl RUNTIME DESTINATION ${LT_BIN})

add_executable (ltsum ltsum.c)
target_link_libraries (ltsum ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltsum RUNTIME DESTINATION ${LT_BIN})

add_executable (ltcam2proj ltcam2proj.c cam2proj.c)
target_link_libraries (ltcam2proj ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltcam2proj RUNTIME DESTINATION ${LT_BIN})
add_executable (ltcam2proj-noise ltcam2proj-noise.c)
target_link_libraries (ltcam2proj-noise ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltcam2proj-noise RUNTIME DESTINATION ${LT_BIN})

add_executable (ltuv2proj ltuv2proj.c)
target_link_libraries (ltuv2proj ${IMGU_LIBS} ${LT_LIBS})
install(TARGETS ltuv2proj RUNTIME DESTINATION ${LT_BIN})

