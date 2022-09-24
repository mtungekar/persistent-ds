# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import copy
import os
from stat import S_IRUSR, S_IRGRP, S_IROTH, S_IWUSR
import importlib

class BaseType:
	def __init__(self,name,variants):
		self.name = name
		self.variants = variants

class BaseTypeVariant:
	def __init__(self,implementing_type,item_type,num_items_per_object,override=False):
		self.implementing_type = implementing_type
		self.item_type = item_type
		self.num_items_per_object = num_items_per_object
		self.overrides_type = override

class ContainerType:
	def __init__(self,container_id,implementing_type,is_template = True):
		self.container_id = container_id
		self.implementing_type = implementing_type
		self.is_template = is_template

base_type_Bool = BaseType('Bool',[BaseTypeVariant('bool','bool',1)])
base_type_Int =  BaseType('Int',[BaseTypeVariant('i8','i8',1),BaseTypeVariant('i16','i16',1),BaseTypeVariant('i32','i32',1),BaseTypeVariant('i64','i64',1)])
base_type_UInt =  BaseType('UInt',[BaseTypeVariant('u8','u8',1),BaseTypeVariant('u16','u16',1),BaseTypeVariant('u32','u32',1),BaseTypeVariant('u64','u64',1)])
base_type_Float =  BaseType('Float',[BaseTypeVariant('float','float',1),BaseTypeVariant('double','double',1)])
base_type_Vec2 =  BaseType('Vec2',[BaseTypeVariant('fvec2','float',2),BaseTypeVariant('dvec2','double',2)]) 
base_type_Vec3 =  BaseType('Vec3',[BaseTypeVariant('fvec3','float',3),BaseTypeVariant('dvec3','double',3)])  
base_type_Vec4 =  BaseType('Vec4',[BaseTypeVariant('fvec4','float',4),BaseTypeVariant('dvec4','double',4)])  
base_type_IVec2 =  BaseType('IVec2',[BaseTypeVariant('i8vec2','i8',2),BaseTypeVariant('i16vec2','i16',2),BaseTypeVariant('i32vec2','i32',2),BaseTypeVariant('i64vec2','i64',2)])    
base_type_IVec3 =  BaseType('IVec3',[BaseTypeVariant('i8vec3','i8',3),BaseTypeVariant('i16vec3','i16',3),BaseTypeVariant('i32vec3','i32',3),BaseTypeVariant('i64vec3','i64',3)])    
base_type_IVec4 =  BaseType('IVec4',[BaseTypeVariant('i8vec4','i8',4),BaseTypeVariant('i16vec4','i16',4),BaseTypeVariant('i32vec4','i32',4),BaseTypeVariant('i64vec4','i64',4)])    
base_type_UVec2 =  BaseType('UVec2',[BaseTypeVariant('u8vec2','u8',2),BaseTypeVariant('u16vec2','u16',2),BaseTypeVariant('u32vec2','u32',2),BaseTypeVariant('u64vec2','u64',2)])    
base_type_UVec3 =  BaseType('UVec3',[BaseTypeVariant('u8vec3','u8',3),BaseTypeVariant('u16vec3','u16',3),BaseTypeVariant('u32vec3','u32',3),BaseTypeVariant('u64vec3','u64',3)])    
base_type_UVec4 =  BaseType('UVec4',[BaseTypeVariant('u8vec4','u8',4),BaseTypeVariant('u16vec4','u16',4),BaseTypeVariant('u32vec4','u32',4),BaseTypeVariant('u64vec4','u64',4)])    
base_type_Mat2 =  BaseType('Mat2',[BaseTypeVariant('fmat2','float',4),BaseTypeVariant('dmat2','double',4)])    
base_type_Mat3 =  BaseType('Mat3',[BaseTypeVariant('fmat3','float',9),BaseTypeVariant('dmat3','double',9)])    
base_type_Mat4 =  BaseType('Mat4',[BaseTypeVariant('fmat4','float',16),BaseTypeVariant('dmat4','double',16)])
base_type_Quat =  BaseType('Quat',[BaseTypeVariant('fquat','float',4),BaseTypeVariant('dquat','double',4)])
base_type_Uuid =  BaseType('Uuid',[BaseTypeVariant('uuid','uuid',1),BaseTypeVariant('item_ref','uuid',1,True)])
base_type_Hash =  BaseType('Hash',[BaseTypeVariant('hash','hash',1),BaseTypeVariant('entity_ref','hash',1,True)])
base_type_String = BaseType('String',[BaseTypeVariant('string','string',1)])

base_types = [base_type_Bool,
			  base_type_Int,
			  base_type_UInt,
			  base_type_Float,
			  base_type_Vec2,
			  base_type_Vec3,
			  base_type_Vec4,
			  base_type_IVec2,
			  base_type_IVec3,
			  base_type_IVec4,
			  base_type_UVec2,
			  base_type_UVec3,
			  base_type_UVec4,
			  base_type_Mat2,
			  base_type_Mat3,
			  base_type_Mat4,
			  base_type_Quat,
			  base_type_Uuid,
			  base_type_Hash,
			  base_type_String]

container_types = [ContainerType(0x00,'none',False),
				   ContainerType(0x01,'optional_value'),
				   ContainerType(0x10,'vector'),
				   ContainerType(0x11,'optional_vector'),
				   ContainerType(0x20,'idx_vector'),
				   ContainerType(0x21,'optional_idx_vector')]

# find a base type based on name, and return base_type and base_type_variant info
def get_base_type_variant( name ):
	for typ in base_types:
		for var in typ.variants:
			if name == var.implementing_type:
				return typ, var
	return None,None

# print all lines using all items in list, with all base types and all variants (including optional variants), as well as all vector versions of base types
def generate_lines_for_all_basetype_combos( line_list ):
	lines = []
	for basetype in base_types:
		for var in basetype.variants:
			for cont in container_types:
				for line in line_list:
					if( cont.is_template ):
						base_type_container_combo = f'{cont.implementing_type}<{var.implementing_type}>'
					else:
						base_type_container_combo = var.implementing_type
					lines.append( 
						line.format( 
							base_type_name = basetype.name , 
							implementing_type = var.implementing_type , 
							container_type = cont.implementing_type , 
							item_type = var.item_type , 
							num_items_per_object = var.num_items_per_object , 
							base_type_combo = base_type_container_combo 
							) 
						)
	return lines

# call function with all base types and all variants (including optional variants), as well as all vector versions of base types
def function_for_all_basetype_combos( line_function ):
	lines = []
	for basetype in base_types:
		for var in basetype.variants:
			for cont in container_types:
				if( cont.is_template ):
					base_type_container_combo = f'{cont.implementing_type}<{var.implementing_type}>'
				else:
					base_type_container_combo = var.implementing_type
				lines.extend( 
					line_function( 
						base_type_name = basetype.name , 
						implementing_type = var.implementing_type , 
						container_type = cont.implementing_type , 
						item_type = var.item_type , 
						num_items_per_object = var.num_items_per_object , 
						base_type_combo = base_type_container_combo 
						) 
					)
	return lines

# reads a file and output lines
def inline_file( path ):
	inlined_file = open(path, 'r')
	lines = inlined_file.readlines()
	strip_lines = []
	for str in lines:
		strip_lines.append( str.rstrip() )
	inlined_file.close()
	return strip_lines

def generate_header():
	lines = []
	lines.append('// WARNING! DO NOT EDIT THIS FILE! This file is generated.')
	lines.append('')
	lines.append('// Generated by pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl')
	lines.append('// Generator code licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE')
	lines.append('')
	return lines

def run_module( name , *args ):
	print('Running: ' + name )
	importlib.import_module('Generators.' + name ).run( *args )
	print('')

def write_lines_to_file( path , lines ):
	# make into one long string
	new_text = ''
	for line in lines:
		new_text += line + '\n'

	# if the file already exists, check if we have an update
	if os.path.exists(path):
		# try reading in the current file if one exists
		with open(path,'r') as f:
			existing_text = f.read()
			f.close()

		#if no difference was found, return
		if new_text == existing_text:
			print( '\tSkipping: ' + path + ', (it is identical)...')
			return

		# if a difference was found, remove the old file
		os.chmod(path, S_IWUSR)
		os.remove( path ) 

	# we should write to the new file
	print( '\tWriting: ' + path + '...')
	with open(path,'w') as f:
		f.write(new_text)
		f.close()

	# change mode of file to readonly
	os.chmod(path, S_IRUSR|S_IRGRP|S_IROTH)