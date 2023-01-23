# set library name
set(PDS_LIB_NAME "pds")

# add as an interface library
add_library(${PDS_LIB_NAME} INTERFACE)

# setup public include headers
target_include_directories(${PDS_LIB_NAME} 
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:pds>
)

# setup commmon gnu install dirs (bin,include, share, lib and etc)
include(GNUInstallDirs)

# setup install
install(
    TARGETS ${PDS_LIB_NAME} EXPORT ${PDS_LIB_NAME}-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
        
install(
    EXPORT ${PDS_LIB_NAME}-config
    NAMESPACE ${PDS_LIB_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PDS_LIB_NAME}
)
             
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pds
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)