#######################################################################
# Define some useful macros/functions that are use throughout
# the cmake scripts
#######################################################################

# NAME: SET_TARGET_OUTPUT_DIRECTORY
# Change the output directory of a library target. Handles the incorrect 
# behaviour of MSVC
#   Parameters:
#      TARGET - The name of the target
#      OUTPUT_DIR - The directory to output the target
#   Optional:
#      An extension to use other than the standard (only used for MSVC)
function( SET_TARGET_OUTPUT_DIRECTORY TARGET OUTPUT_DIR )
  get_target_property(TARGET_NAME ${TARGET} OUTPUTNAME)
  if ( MSVC )
      # For some reason when the generator is MSVC  10 it ignores the LIBRARY_OUTPUT_DIRECTORY
      # property so we have to do something slightly different
      if ( ${ARGC} STREQUAL 3 )
        set ( LIB_EXT ${ARGV2} )
      else ()
        set ( LIB_EXT .dll )
      endif()
      set ( SRC_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR} )
      add_custom_command( TARGET ${TARGET} POST_BUILD 
                          COMMAND ${CMAKE_COMMAND} ARGS -E make_directory
                          ${OUTPUT_DIR} )
      add_custom_command ( TARGET ${TARGET} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} ARGS -E echo 
                           "Copying \"$(TargetName)${LIB_EXT}\" to ${OUTPUT_DIR}/$(TargetName)${LIB_EXT}"
                           COMMAND ${CMAKE_COMMAND} ARGS -E copy
                           ${SRC_DIR}/$(TargetName)${LIB_EXT}
                           ${OUTPUT_DIR}/$(TargetName)${LIB_EXT} )
      # Clean up
      set ( LIB_EXT )
      set ( SRC_DIR )
  else ()
    set_target_properties ( ${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR} )
  endif ( MSVC )
endfunction( SET_TARGET_OUTPUT_DIRECTORY )

#######################################################################

#######################################################################

#
# NAME: COPY_PYTHON_FILES_TO_DIR
# Adds a set of custom commands for each python file to copy
# the given file to the destination directory
#   - PY_FILES :: A list of python files to copy. Note you will have
#                 to quote an expanded list
#   - SRC_DIR :: The src directory of the files to be copied
#   - DEST_DIR :: The final directory for the copied files
#   - INSTALLED_FILES :: An output variable containing the list of copied
#                        files including their full paths
function( COPY_PYTHON_FILES_TO_DIR PY_FILES SRC_DIR DEST_DIR INSTALLED_FILES )
    set ( COPIED_FILES ${${INSTALLED_FILES}} )
    foreach ( PYFILE ${PY_FILES} )
        get_filename_component( _basefilename ${PYFILE} NAME_WE )
        set( _py_src ${SRC_DIR}/${PYFILE} )
        set( _py_bin ${DEST_DIR}/${PYFILE} )
        add_custom_command ( OUTPUT ${_py_bin}
                             DEPENDS ${_py_src}
                             COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different
                               ${_py_src} ${_py_bin} )
    set ( COPIED_FILES ${COPIED_FILES} ${_py_bin} )
    endforeach ( PYFILE )
    set ( ${INSTALLED_FILES} ${COPIED_FILES} PARENT_SCOPE )
endfunction( COPY_PYTHON_FILES_TO_DIR )

#######################################################################

# NAME: CLEAN_ORPHANED_PYC_FILES
# Given a directory, glob for .pyc files and if there is not a .py file
# next to it. Assumes that the .pyc was not under source control. Note
# that the removal happens when cmake is run
#   Parameters:
#      ROOT_DIR - The root directory to start the search (recursive)
function ( CLEAN_ORPHANED_PYC_FILES ROOT_DIR )
  file ( GLOB_RECURSE PYC_FILES ${ROOT_DIR}/*.pyc )
  if ( PYC_FILES )
    foreach ( PYC_FILE ${PYC_FILES} )
      get_filename_component ( DIR_COMP ${PYC_FILE} PATH )
      get_filename_component ( STEM ${PYC_FILE} NAME_WE )
      set ( PY_FILE ${DIR_COMP}/${STEM}.py )
      if ( NOT EXISTS ${PY_FILE} )
        execute_process ( COMMAND ${CMAKE_COMMAND} -E remove -f ${PYC_FILE} )
      endif()
    endforeach()
  endif()
endfunction()
