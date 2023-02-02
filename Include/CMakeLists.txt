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

# public header file set
set(PUBLIC_HEADER_SET
    pds/BidirectionalMap.h
    pds/DataTypes.h
    pds/DataTypes.inl
    pds/DataValuePointers.h
    pds/DirectedGraph.h
    pds/DynamicTypes.h
    pds/DynamicTypes.inl
    pds/EntityReader.h
    pds/EntityReader.inl
    pds/EntityReaderTemplates.inl
    pds/EntityValidator.h
    pds/EntityWriter.h
    pds/EntityWriter.inl
    pds/EntityWriterTemplates.inl
    pds/IndexedVector.h
    pds/ItemTable.h
    pds/Log.h
    pds/MemoryReadStream.h
    pds/MemoryWriteStream.h
    pds/pds.h
    pds/pds.inl
    pds/SHA256.h
    pds/SHA256.inl
    pds/Varying.h
    pds/Varying.inl
)

# setup public headers
set_target_properties(${PDS_LIB_NAME} PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADER_SET}"
        
)

# setup commmon gnu install dirs (bin,include, share, lib and etc)
include(GNUInstallDirs)

# setup install
install(
    TARGETS ${PDS_LIB_NAME} 
    EXPORT ${PDS_LIB_NAME}-config
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pds
)
        
install(
    EXPORT ${PDS_LIB_NAME}-config
    NAMESPACE ${PDS_LIB_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PDS_LIB_NAME}
)