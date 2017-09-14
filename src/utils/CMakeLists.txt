add_library (ltutils graycode.c gamma.c ltsys.c noise.cpp triangle.c uvmap.c entry.c pd_send.c Thread.cpp ThreadInterface.cpp)
target_link_libraries (ltutils ${BMC_LIBS})
#install(TARGETS ltutils ARCHIVE DESTINATION ${LT_LIB})

