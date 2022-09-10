// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <pds/Varying.h>
#include <pds/EntityWriter.h>
#include <pds/EntityReader.h>
#include <pds/EntityValidator.h>

using namespace pds;

Varying::Varying( const Varying &rval )
	{
	this->type_m = {};
	this->container_type_m = {};
	this->data_m = {};

	Varying::MF::DeepCopy( *this, &rval );
	}

Varying &Varying::operator=( const Varying &rval )
	{
	Varying::MF::DeepCopy( *this, &rval );
	return *this;
	}

Varying::Varying( Varying &&rval ) noexcept
	{
	this->type_m = rval.type_m;
	rval.type_m = {};

	this->container_type_m = rval.container_type_m;
	rval.container_type_m = {};

	this->data_m = rval.data_m;
	rval.data_m = {};
	}

Varying &Varying::operator=( Varying &&rval ) noexcept
	{
	this->type_m = rval.type_m;
	rval.type_m = {};

	this->container_type_m = rval.container_type_m;
	rval.container_type_m = {};

	this->data_m = rval.data_m;
	rval.data_m = {};

	return *this;
	}

Varying::~Varying()
	{
	this->Deinitialize();
	}

bool Varying::operator==( const Varying &rval ) const
	{
	return MF::Equals( this, &rval );
	};

bool Varying::operator!=( const Varying &rval ) const
	{
	return !(operator==( rval ));
	}

bool Varying::Deinitialize()
	{
	// delete allocated data if nonempty
	if( this->IsInitialized() )
		{
		bool success = dynamic_types::delete_type( this->type_m, this->container_type_m, this->data_m );
		if( !success )
			{
			pdsErrorLog << "Error in call to dynamic_types::delete_type" << pdsErrorLogEnd;
			return false;
			}

		// clear values
		this->type_m = {};
		this->container_type_m = {};
		this->data_m = {};
		}

	return true;
	}

bool Varying::Initialize( data_type_index dataType, container_type_index containerType )
	{
	return MF::SetType( *this, dataType, containerType );
	}

std::tuple<data_type_index, container_type_index> Varying::Type() const noexcept
	{
	return std::pair<data_type_index, container_type_index>( this->type_m, this->container_type_m );
	}

// Returns true if the object has been initialized (ie has a data object)
bool Varying::IsInitialized() const noexcept
	{
	return this->data_m != nullptr;
	}

void Varying::MF::Clear( Varying &obj )
	{
	if( obj.data_m == nullptr )
		return;
	bool success = dynamic_types::clear( obj.type_m, obj.container_type_m, obj.data_m );
	pdsRuntimeCheck( success, Status::EUndefined, "Could not clear the dynamic type" );
	}

void Varying::MF::DeepCopy( Varying &dest, const Varying *source )
	{
	bool success = {};

	// clear the current type of dest
	success = dest.Deinitialize();
	pdsRuntimeCheck( success, Status::EUndefined, "Could not clear Varying type" );

	// if source is nullptr or empty we are done
	if( !source || !source->IsInitialized() )
		return;

	// set type and clear any currently allocated data
	success = SetType( dest, source->type_m, source->container_type_m );
	pdsRuntimeCheck( success, Status::EUndefined, "Cannot set Varying type." );

	// copy the data from source to dest
	success = dynamic_types::copy( dest.type_m, dest.container_type_m, dest.data_m, source->data_m );
	pdsRuntimeCheck( success, Status::EUndefined, "Cannot copy Varying type data." );
	}

bool Varying::MF::Equals( const Varying *lvar, const Varying *rvar )
	{
	// early out if the pointers are equal (includes nullptr)
	if( lvar == rvar )
		return true;

	// early out if one of the pointers is nullptr (both can't be null because of above test)
	if( !lvar || !rvar )
		return false;

	// both pointers are valid and different pointers, compare type data
	if( lvar->type_m != rvar->type_m )
		return false;
	if( lvar->container_type_m != rvar->container_type_m )
		return false;

	// types match, compare data in type
	return dynamic_types::equals( lvar->type_m, lvar->container_type_m, lvar->data_m, rvar->data_m );
	}

bool Varying::MF::Write( const Varying &obj, EntityWriter &writer )
	{
	if( !obj.IsInitialized() )
		{
		pdsErrorLog << "Cannot write uninitialized Varying to stream. Use optional_value template for optional Varying data." << pdsErrorLogEnd;
		return false;
		}

	// store the data type index
	if( !writer.Write( pdsKeyMacro( "Type" ), (u16)obj.type_m ) )
		return false;

	// store the container type index
	if( !writer.Write( pdsKeyMacro( "ContainerType" ), (u16)obj.container_type_m ) )
		return false;

	// store the data
	if( !dynamic_types::write( obj.type_m, obj.container_type_m, pdsKeyMacro( "Data" ), writer, obj.data_m ) )
		return false;

	return true;
	}

bool Varying::MF::Read( Varying &obj, EntityReader &reader )
	{
	// if initialized, deinitalize
	obj.Deinitialize();

	u16 type_u16 = {};
	u16 container_type_u16 = {};

	// load the data type index
	if( !reader.Read( pdsKeyMacro( "Type" ), type_u16 ) )
		return false;

	// load the container type index
	if( !reader.Read( pdsKeyMacro( "ContainerType" ), container_type_u16 ) )
		return false;

	// set the data type
	if( !SetType( obj, (data_type_index)type_u16, (container_type_index)container_type_u16 ) )
		return false;

	// store the data
	if( !dynamic_types::read( obj.type_m, obj.container_type_m, pdsKeyMacro( "Data" ), reader, obj.data_m ) )
		return false;

	return true;
	}

bool Varying::MF::Validate( const Varying &obj, EntityValidator &validator )
	{
	if( !obj.IsInitialized() )
		{
		pdsValidationError( ValidationError::NullNotAllowed )
			<< "Object is not initialized, and does not have a type set. All Varying objects need to be initialized to be valid. To have an optional Varying object, use the optional_value template."
			<< pdsValidationErrorEnd;
		}

	return true;
	}

bool Varying::MF::SetType( Varying &obj, data_type_index dataType, container_type_index containerType )
	{
	bool success = {};

	// clear current type if it is set
	success = obj.Deinitialize();
	if( !success )
		{
		pdsErrorLog << "Error in call to Varying::Deinitialize" << pdsErrorLogEnd;
		return false;
		}

	// set type and allocate the data
	obj.type_m = dataType;
	obj.container_type_m = containerType;
	std::tie( obj.data_m, success ) = dynamic_types::new_type( obj.type_m, obj.container_type_m );

	return success;
	}
