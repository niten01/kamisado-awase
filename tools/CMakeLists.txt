if(BUILD_GUI)
  add_subdirectory(engine-gui)
endif()

if(BUILD_SERVER)
  add_subdirectory(web/server)
endif()
