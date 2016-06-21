add_sourcekit_executable(suptool
  suptool.cpp
  DEPENDS 
  SourceKitSupport SourceKitSwiftLang
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(suptool
    PROPERTIES
    LINK_FLAGS "-Wl,-rpath -Wl,@executable_path/../lib")

endif()

swift_install_in_component(tools
    TARGETS suptool
    RUNTIME DESTINATION bin)
