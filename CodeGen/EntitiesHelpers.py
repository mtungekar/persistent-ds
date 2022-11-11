# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

from __future__ import annotations
import CodeGeneratorHelpers as hlp
import sys

# class types which are built in to pds
pds_built_in_item_types = {
	'BidirectionalMap',
	'DirectedGraph',
	'IndexedVector',
	'ItemTable',
	'Varying'
}

class Dependency:
	"""definition of a dependency item/entity in the package or a built-in type from pds"""	
	def __init__(self, name, include_in_header = False ):
		self.Name = name
		self.IncludeInHeader = include_in_header
		self.PDSType = name in pds_built_in_item_types

class Template:
	"""implementation of a template class"""
	def __init__(self, name, template, types, flags = [] ):
		self.Name = name
		self.Template = template
		self.Types = types
		self.Flags = flags

		# create declaration of template
		self.Declaration = f'            using {self.Name} = pds::{self.Template}<'
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
				self.Declaration += 'pds::' + self.Template + 'Flags::' + fl
				has_flag = True
			has_value = True
		self.Declaration += '>;'

class Variable:
	"""a variable in the item/entity"""
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

class Validation:
	"""Validation type"""

class ValidateAllKeysAreInTable(Validation):
	"""validation that makes sure all keys in one table or map are keys in another table"""

	def __init__(self, tableToValidate:str, mustExistInTable:str):
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

class Mapping:
	"""Base class of variable mappings between versions of items"""
	def __init__(self, variables: list[str] ):
		self.Variables = variables

class NewVariable(Mapping):
	"""definition of a new variable which has no corresponding variable in the previous version"""
	def __init__(self, name:str ):
		super().__init__( [name] )

class DeletedVariable(Mapping):
	"""definition of a variable which has been removed in this version of the item"""
	def __init__(self, previousName:str ):
		super().__init__( [] )
		self.PreviousName = previousName		

class RenamedVariable(Mapping):
	"""copy a value from one variable name to another. this is useful for renamed variables"""
	def __init__(self, name:str , previousName:str ):
		super().__init__( [name] )
		self.PreviousName = previousName

class SameVariable(RenamedVariable):
	"""same variable in modified version"""
	def __init__(self, name:str ):
		super().__init__(name,name)

class CustomCodeMapping(Mapping):
	"""custom code implemented to copy the values to and from the previous version. this mapping can cover multiple named variables."""
	def __init__(self, variables: list[str], toPrevious:str, fromPrevious:str ):
		super().__init__(variables)
		self.ToPrevious = toPrevious
		self.FromPrevious = fromPrevious

class Item:
	"""item base class"""
	def __init__(self, name:str ):
		self.Name = name
		self.IsEntity = False
		self.IdenticalToPreviousVersion = False
		self.IsDeleted = False
		self.IsDeprecated = False
		self.IsModifiedFromPreviousVersion = False

class NewItem(Item):
	"""definition of a new item, which does not exist in the previous version"""
	def __init__(self, name:str, variables:list[Variable], dependencies:list[Dependency] = [], templates:list[Template] = [], validations:list[Validation] = [] ):
		super().__init__(name)
		self.Dependencies = dependencies
		self.Templates = templates
		self.Variables = variables
		self.Validations = validations

	def FindDependency( self, name ):
		return next(val for val in self.Dependencies if val.Name == name )

	def FindTemplate( self, name ):
		return next(val for val in self.Templates if val.Name == name )

	def FindVariable( self, name ):
		return next(val for val in self.Variables if val.Name == name )

class NewEntity(NewItem):
	"""entity which has no entity in an earlier version which it is derived from"""
	def __init__(self, name:str, variables:list[Variable], dependencies:list[Dependency] = [], templates:list[Template] = [], validations:list[Validation] = [] ):
		super().__init__( name, variables, dependencies, templates, validations )
		self.IsEntity = True

class IdenticalItem(Item):
	"""an item which is identical to the item in the previous package"""
	def __init__(self, name:str):
		super().__init__( name )
		self.IdenticalToPreviousVersion = True

class IdenticalEntity(IdenticalItem):
	"""an entity which is identical to the item in the previous package"""
	def __init__(self, name:str):
		super().__init__( name )
		self.IsEntity = True

class DeletedItem(Item):
	"""an item which is deleted in this version of the package"""
	def __init__(self, name:str):
		super().__init__( name )
		self.IsDeleted = True

class DeletedEntity(DeletedItem):
	"""an entity which is deleted in this version of the package"""
	def __init__(self, name:str):
		super().__init__( name )
		self.IsEntity = True

class ModifiedItem(NewItem):
	"""an item which is modified in this version of the package"""
	def __init__(self, name:str, variables:list[Variable], dependencies:list[Dependency] = [], templates:list[Template] = [], validations:list[Validation] = [], mappings:list[Mapping] = [] ):
		super().__init__( name, variables, dependencies, templates, validations )
		self.IsModifiedFromPreviousVersion = True
		self.Mappings = mappings

class ModifiedEntity(ModifiedItem):
	"""an entity which is modified in this version of the package"""
	def __init__(self, name:str, variables:list[Variable], dependencies:list[Dependency] = [], templates:list[Template] = [], validations:list[Validation] = [], mappings:list[Mapping] = [] ):
		super().__init__( name, variables, dependencies, templates, validations, mappings )
		self.IsEntity = True

class Version:
	"""A specific version of a package, which defines all items which are available in that version"""

	# versionName - name of the version, use whatever convention you want, but needs to start with a letter, it will be the namespace of the version in the code
	# previousVersion - reference the version that this version is a direct update from
	# items - all the items in the version, including any item which is inherited from a previous version
	def __init__(self, name:str, previousVersion:Version, items:list[Item] ):
		self.Name = name
		self.Items = items
		self.PreviousVersion = previousVersion

		# set the version reference in each item
		for item in self.Items:
			item.Version = self

class Package:
	"""The package of a project using pds"""

    # name - name of package (including any versioning)
    # parentPath - name of the folder where the package will be placed
	def __init__(self, name:str, path:str, versions:list[Version] ):
		self.Name = name
		self.Versions = versions
		self.Path = path
		self.SetupReferences()
		self.SetupPreviousVersionsOfItems()
		self.MakeSureAllItemsAreDefined()
		self.MakeSureAllVariablesAreMapped()

	# setup references to the package in all items and versions
	def SetupReferences(self) -> None:
		for version in self.Versions:
			version.Package = self
			for item in version.Items:
				item.Package = self

	# for each version of the package, if it has a previous version
	# make sure all items in the previous version exists in this
	# version, unless it was deleted in the previous version
	def MakeSureAllItemsAreDefined(self) -> None:
		for version in self.Versions:
			prevVersion = version.PreviousVersion
			if prevVersion != None:
				for prevItem in prevVersion.Items:
					if not prevItem.IsDeleted:
						itmFnd = next( (itm for itm in version.Items if itm.Name == prevItem.Name ) , None )
						if itmFnd == None:
							raise Exception(f"Invalid setup in package {self.Name}, the item {prevItem.Name} in version {prevVersion.Name} is not defined in subsequent version {version.Name}")
						if itmFnd.IsEntity != prevItem.IsEntity:
							raise Exception(f"Invalid setup in package {self.Name}, the item {prevItem.Name} in version {prevVersion.Name} is not of the same Item/Entity type in subsequent version {version.Name}")

	# find all previous versions of items				
	def SetupPreviousVersionsOfItems(self) -> None:
		for version in self.Versions:
			for item in version.Items:
				if item.IdenticalToPreviousVersion or item.IsModifiedFromPreviousVersion:
					version = item.Version
					while version.PreviousVersion != None:
						version = version.PreviousVersion
						itmFnd = next( (itm for itm in version.Items if itm.Name == item.Name and not itm.IdenticalToPreviousVersion) , None )
						if itmFnd != None:
							if itmFnd.IsEntity != item.IsEntity:
								raise Exception(f"FindActualItem: In Package: {self.Name}, Version: {item.Version.Name} Found previous a item of the name '{item.Name}', but that item has IsEntity={itmFnd.IsEntity()} which does not match this item's IsEntity={itmFnd.IsEntity()}")
							item.PreviousVersion = itmFnd
							break
					if item.PreviousVersion == None:
						raise Exception(f"Invalid setup, the item {item.Name} in Package: {self.Name}, Version: {item.Version.Name} is not correctly setup, no previous version of the item is found in any package") 

	# for each item which is a modified version, make sure all variables are handled by mapping
	def MakeSureAllVariablesAreMapped(self) -> None:
		for version in self.Versions:
			for item in version.Items:
				if item.IsModifiedFromPreviousVersion:

					# collect all mappings, and check that all variables are covered
					coveredVariables = set()
					for mapping in item.Mappings:
						for variable in mapping.Variables:
							coveredVariables.add( variable )

					# make sure all variables are mapped in the set
					for variable in item.Variables:
						if variable.Name not in coveredVariables:
							raise Exception(f'Variable {variable.Name} is not handled by a mapping in version {version.Name} of item {item.Name}')
	