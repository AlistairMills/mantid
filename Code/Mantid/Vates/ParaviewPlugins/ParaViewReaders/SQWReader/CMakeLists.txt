project( MantidParaViewSQWReader )

add_paraview_plugin( MantidParaViewSQWReaderSMPlugin "1.0"
	SERVER_MANAGER_XML SQWReader.xml
	SERVER_MANAGER_SOURCES vtkSQWReader.cxx
    GUI_RESOURCE_FILES SQWReaderGUI.xml
)

# Add to the 'VatesParaViewPlugins' group in VS
set_property( TARGET MantidParaViewSQWReaderSMPlugin PROPERTY FOLDER "MantidVatesParaViewPlugins" )

target_link_libraries( MantidParaViewSQWReaderSMPlugin 
${MANTID_SUBPROJECT_LIBS} )

# Put library into subfolder.
SET_TARGET_OUTPUT_DIRECTORY( MantidParaViewSQWReaderSMPlugin ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${PVPLUGINS_DIR}/${PVPLUGINS_SUBDIR})

install( TARGETS MantidParaViewSQWReaderSMPlugin ${SYSTEM_PACKAGE_TARGET} DESTINATION ${PVPLUGINS_DIR}/${PVPLUGINS_SUBDIR} )



