// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"

namespace pds
    {
    class MemoryReadStream;
    class MemoryWriteStream;
    class EntityWriter;
    class EntityReader;
    
    namespace dynamic_types
        { 
        // dynamically allocate a data of data type and container combination
        std::tuple<void*, bool> new_type( data_type_index dataType , container_type_index containerType );
    
        // delete a previously allocated data object
        // caveat: no type checking is done, so make sure to supply the correct 
        // type combo to the function.
        bool delete_type( data_type_index dataType , container_type_index containerType , void *data );
    
        // clear the contents for data object.
        // caveat: no type checking is done, so make sure to supply the correct 
        // type combo to the function.
        bool clear( data_type_index dataType , container_type_index containerType , void *data );
    
        // write the data to an entity writer stream.
        // caveat: no type checking is done, so make sure to supply the correct 
        // type combo to the function.
        bool write( data_type_index dataType , container_type_index containerType , const char *key, const u8 key_length , EntityWriter &writer , const void *data );
    
        // read the data to from an entity reader stream.
        // caveat: no type checking is done, so make sure to supply the correct 
        // type combo to the function.
        bool read( data_type_index dataType , container_type_index containerType , const char *key, const u8 key_length , EntityReader &reader , void *data );
    
        // copy data from src to dest. both data object must be the same type and be allocated.
        // caveat: no type checking is done, so make sure to supply the correct 
        // type combo to the function.
        bool copy( data_type_index dataType , container_type_index containerType , void *dest , const void *src );
    
        // check if two data objects have the same internal data
        // caveat: no type checking is done, so make sure to supply the correct 
        // type combo to the function.
        bool equals( data_type_index dataType , container_type_index containerType , const void *dataA , const void *dataB );
    
        };
    };
