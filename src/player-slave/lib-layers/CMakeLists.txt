add_library (player_layers SHARED ltvideotexture.cpp ltlayer.cpp lt3dlayer.cpp lttileinfo.cpp ltvideocontroller.cpp lttiledvideolayer.cpp ltmulti3dlayer.cpp ltcapturelayer.cpp ltcolorlayer.cpp ltvideolayer.cpp)
target_link_libraries (player_layers ${IMGU_LIBS} ${LT_LIBS} ${OSG_LIBS} ${AV_LIBS} ${BMC_LIBS})
install(TARGETS player_layers LIBRARY DESTINATION ${LT_LIB})

install(FILES deform_video.glsl DESTINATION ${LT_SHARE})
install(FILES deform_3d.glsl DESTINATION ${LT_SHARE})
install(FILES stereo_rotate_vert.glsl DESTINATION ${LT_SHARE})

