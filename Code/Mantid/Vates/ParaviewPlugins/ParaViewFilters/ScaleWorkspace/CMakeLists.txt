PROJECT(ScaleWorkspace)

ADD_PARAVIEW_PLUGIN(MantidParaViewScaleWorkspaceSMPlugin "1.0"
SERVER_MANAGER_XML ScaleWorkspace.xml
SERVER_MANAGER_SOURCES vtkScaleWorkspace.cxx
GUI_RESOURCE_FILES ScaleWorkspaceGUI.xml)
# Add to the 'VatesParaViewPlugins' group in VS
set_property( TARGET MantidParaViewScaleWorkspaceSMPlugin PROPERTY FOLDER "MantidVatesParaViewPlugins")

target_link_libraries( MantidParaViewScaleWorkspaceSMPlugin
${MANTID_SUBPROJECT_LIBS} )

# Put library into subfolder.
SET_TARGET_OUTPUT_DIRECTORY(MantidParaViewScaleWorkspaceSMPlugin  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${PVPLUGINS_DIR}/${PVPLUGINS_SUBDIR})

install( TARGETS MantidParaViewScaleWorkspaceSMPlugin ${SYSTEM_PACKAGE_TARGET} DESTINATION ${PVPLUGINS_DIR}/${PVPLUGINS_SUBDIR})
