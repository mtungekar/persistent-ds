# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

class Item:
	def __init__(self, name, variables, dependencies = [], templates = [], validations = [] ):
		self.Name = name
		self.Dependencies = dependencies
		self.Templates = templates
		self.Variables = variables
		self.Validations = validations
		self.IsEntity = False

	def FindDependency( self, name ):
		return next(val for val in self.Dependencies if val.Name == name )

	def FindTemplate( self, name ):
		return next(val for val in self.Templates if val.Name == name )

	def FindVariable( self, name ):
		return next(val for val in self.Variables if val.Name == name )

class Entity(Item):
	def __init__(self, name, variables, dependencies = [], templates = [], validations = [] ):
		super().__init__( name, variables, dependencies, templates, validations )
		self.IsEntity = True

class Dependency:
	def __init__(self, namespace, name, include_in_header = False ):
		self.Name = name
		self.Namespace = namespace
		self.IncludeInHeader = include_in_header

class Template:
	def __init__(self, name, template, types, flags = [] ):
		self.Name = name
		self.Template = template
		self.Types = types
		self.Flags = flags

		# create declaration of template
		self.Declaration = f'            using {self.Name} = {self.Template}<'
		has_value = False
		for ty in self.Types:
			if has_value:
				self.Declaration += ','
			self.Declaration += ty
			has_value = True
		if len(self.Flags) > 0:
			if has_value:
				self.Declaration += ','
			has_flag = False;
			for fl in self.Flags:
				if has_flag:
					self.Declaration += '|'
				self.Declaration += self.Template + 'Flags::' + fl
				has_flag = True
			has_value = True
		self.Declaration += '>;'

class Variable:
	def __init__(self, type, name, optional = False, vector = False, indexed = False ):
		self.Type = type
		self.Name = name
		self.Optional = optional
		self.Vector = vector
		self.IndexedVector = indexed
		if self.IndexedVector and not self.Vector:
			sys.exit("Variable.__init__: IndexedVector requires Vector flag to be set as well")

		# build the type string
		if self.Optional:
			if self.Vector:
				if self.IndexedVector:
					self.TypeString = f"optional_idx_vector<{self.Type}>"
				else:
					self.TypeString = f"optional_vector<{self.Type}>"
			else:
				self.TypeString = f"optional_value<{self.Type}>"
		else:
			if self.Vector:
				if self.IndexedVector:
					self.TypeString = f"idx_vector<{self.Type}>"
				else:
					self.TypeString = f"std::vector<{self.Type}>"
			else:
				self.TypeString = self.Type

		# check if this is a simple value, or a complex value (which is using one of the wrapper classes)
		self.IsSimpleValue = (not self.Optional) and (not self.Vector) and (not self.IndexedVector)
		self.IsComplexValue = not self.IsSimpleValue

		# look up BaseType and BaseVariant
		self.BaseType,self.BaseVariant = hlp.get_base_type_variant(self.Type)
		self.IsBaseType = self.BaseType is not None

		# check if this is a simple value which is a base type
		self.IsSimpleBaseType = self.BaseType and self.IsSimpleValue

# validation that makes sure all keys in one table or map are keys in another table
class ValidateAllKeysAreInTable:
	def __init__(self, tableToValidate, mustExistInTable):
		self.TableToValidate = tableToValidate
		self.MustExistInTable = mustExistInTable

	def GenerateValidationCode( self, entity, indentation ):
		# find the types of the variables
		tableType = entity.FindVariable( self.TableToValidate ).Type
				
		lines = []
		lines.append( f'{indentation}// Validate that all keys in {self.TableToValidate} also exist in {self.MustExistInTable}' )
		lines.append( f'{indentation}success = {tableType}::MF::ValidateAllKeysAreContainedInTable( obj.v_{self.TableToValidate} , validator , obj.v_{self.MustExistInTable} , "{self.MustExistInTable}" );' )
		lines.append( f'{indentation}if( !success )' )
		lines.append( f'{indentation}    return false;' )

		return lines
