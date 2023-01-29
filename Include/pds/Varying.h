// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "DynamicTypes.h"
#include "ValueTypes.h"

namespace pds
    {
    class Varying
        {
        public:
            class MF;
            friend MF;

            Varying() = default;
            Varying( const Varying &rval );
            Varying &operator=( const Varying &rval );
            Varying( Varying &&rval ) noexcept;
            Varying &operator=( Varying &&rval ) noexcept;
            ~Varying();

            // value compare operators
            bool operator==( const Varying &rval ) const;
            bool operator!=( const Varying &rval ) const;

        protected:
            data_type_index type_m = {};
            container_type_index container_type_m = {};
            void *data_m = {};

            bool Deinitialize();

        public: 
            // Initialize the varying object, and allocate the data
            bool Initialize( data_type_index dataType, container_type_index containerType );
            template <class _Ty> _Ty &Initialize();

            // The current data type in the object
            std::tuple<data_type_index, container_type_index> Type() const noexcept;

            // Returns true if the object has been initialized (ie has a data object)
            bool IsInitialized() const noexcept;

            // Check if the data is of the template class type _Ty
            template<class _Ty> bool IsA() const noexcept;

            // Retreive a reference to the data in the object
            template<class _Ty> const _Ty &Data() const;
            template<class _Ty> _Ty &Data();
        };

    class EntityWriter;
    class EntityReader;
    class EntityValidator;

    class Varying::MF
        {
        public:
            static void Clear( Varying &obj );
            static void DeepCopy( Varying &dest, const Varying *source );
            static bool Equals( const Varying *lvar, const Varying *rvar );

            static bool Write( const Varying &obj, EntityWriter &writer );
            static bool Read( Varying &obj, EntityReader &reader );

            static bool Validate( const Varying &obj, EntityValidator &validator );

            // Method to set the type of the data in the varying object, either using a parameter, or as a template method
            static bool SetType( Varying &obj, data_type_index dataType, container_type_index containerType );
            template <class _Ty> static bool SetType( Varying &obj ) { return SetType( obj, combined_type_information<_Ty>::type_index, combined_type_information<_Ty>::container_index ); }
        };

    template <class _Ty> _Ty & Varying::Initialize()
        {
        bool success = this->Initialize( combined_type_information<_Ty>::type_index, combined_type_information<_Ty>::container_index );
        pdsRuntimeCheck( success, Status::ENotInitialized, "Failed to initialize Varying object" );
        return *((_Ty*)data_m);
        }

    // Check if the data is of the template class type _Ty
    template<class _Ty> bool Varying::IsA() const noexcept 
        { 
        return this->type_m == combined_type_information<_Ty>::type_index 
            && this->container_type_m == combined_type_information<_Ty>::container_index; 
        }

    // Retreive a reference to the data in the object
    template<class _Ty> const _Ty &Varying::Data() const 
        {  
        pdsRuntimeCheck( this->IsInitialized(), Status::ENotInitialized, "Dereferencing non-initialized object" );
        pdsRuntimeCheck( this->IsA<_Ty>(), Status::EInvalid, "Wrong type when dereferencing" );
        return *((const _Ty*)data_m);
        };

    // Retreive a reference to the data in the object
    template<class _Ty> _Ty & Varying::Data() 
        { 
        pdsRuntimeCheck( this->IsInitialized(), Status::ENotInitialized, "Dereferencing non-initialized object" );
        pdsRuntimeCheck( this->IsA<_Ty>(), Status::EInvalid, "Wrong type when dereferencing" );
        return *((_Ty*)data_m);
        };

	};
