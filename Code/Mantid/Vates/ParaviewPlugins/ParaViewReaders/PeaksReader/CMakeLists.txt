project( MantidParaViewPeaksReader )

add_paraview_plugin( MantidParaViewPeaksReaderSMPlugin "1.0"
	SERVER_MANAGER_XML PeaksReader.xml
	SERVER_MANAGER_SOURCES vtkPeaksReader.cxx
	GUI_RESOURCE_FILES PeaksReaderGUI.xml
)

# Add to the 'VatesParaViewPlugins' group in VS
set_property( TARGET MantidParaViewPeaksReaderSMPlugin PROPERTY FOLDER "MantidVatesParaViewPlugins" )

target_link_libraries( MantidParaViewPeaksReaderSMPlugin 
${MANTID_SUBPROJECT_LIBS} )

# Put library into subfolder.
SET_TARGET_OUTPUT_DIRECTORY(MantidParaViewPeaksReaderSMPlugin ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${PVPLUGINS_DIR}/${PVPLUGINS_SUBDIR})

install( TARGETS MantidParaViewPeaksReaderSMPlugin ${SYSTEM_PACKAGE_TARGET} DESTINATION ${PVPLUGINS_DIR}/${PVPLUGINS_SUBDIR} )


