cmake_minimum_required(VERSION 3.21)
project( persistent-ds )

find_package(PythonInterp REQUIRED)
find_package(Python REQUIRED)
include(FetchContent)

# glm 
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG        bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)
			
# ctle 
FetchContent_Declare(
	ctle
	GIT_REPOSITORY https://github.com/Cooolrik/ctle.git
	GIT_TAG		   main 
)

# librocksha256 
FetchContent_Declare( 
	librocksha256
	GIT_REPOSITORY https://github.com/forrestcavalier/librock_sha256.git
	GIT_TAG		   387f4cfff7c5913bcbeaf4c1c1f53f657cc4a6d0
)
	
# generate the pds code
execute_process( 
	COMMAND 			${PYTHON_EXECUTABLE} CodeGenerator.py
	WORKING_DIRECTORY 	${CMAKE_CURRENT_LIST_DIR}/CodeGen
	RESULT_VARIABLE 	py_result
)
message(STATUS "Result of CodeGenerator.py: ${py_result}")
	
# if this is the main build of persistent-ds, build the testing code
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)			
				
	# googletest
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY 	https://github.com/google/googletest.git
		GIT_TAG			58d77fa8070e8cec2dc1ed015d66b454c8d78850 # 1.12.1 
		)
	
	FetchContent_MakeAvailable( 
		glm
		ctle 
		librocksha256 
		googletest 
		)
		
	# generate the pds test code
	execute_process( 
		COMMAND 			${PYTHON_EXECUTABLE} GenerateTestPacks.py
		WORKING_DIRECTORY 	${CMAKE_CURRENT_LIST_DIR}/CodeGen
		RESULT_VARIABLE 	py_result
	)
	message(STATUS "Result of GenerateTestPacks.py: ${py_result}")

	# lots of warnings and all warnings as errors
	# big object files
	if(MSVC)
		add_compile_options(/W4 /WX)
		add_compile_options(/bigobj)
	else()
		add_compile_options(-Wall -Wextra -pedantic -Werror)
	endif()
	set( CMAKE_DEBUG_POSTFIX _d )
	
	# SystemTest run full system tests
	add_executable( 
		systemtest
		./Tests/SystemTest.cpp 
		./Tests/TestPackA/TestPackA.cpp 

		# all pds files
		./Include/pds/BidirectionalMap.h
		./Include/pds/DataTypes.h
		./Include/pds/DataTypes.inl
		./Include/pds/DataValuePointers.h
		./Include/pds/DirectedGraph.h
		./Include/pds/DynamicTypes.h
		./Include/pds/DynamicTypes.inl
		./Include/pds/EntityReader.h
		./Include/pds/EntityReader.inl
		./Include/pds/EntityReaderTemplates.inl
		./Include/pds/EntityValidator.h
		./Include/pds/EntityWriter.h
		./Include/pds/EntityWriter.inl
		./Include/pds/EntityWriterTemplates.inl
		./Include/pds/IndexedVector.h
		./Include/pds/ItemTable.h
		./Include/pds/Log.h
		./Include/pds/MemoryReadStream.h
		./Include/pds/MemoryWriteStream.h
		./Include/pds/pds.h
		./Include/pds/pds.inl
		./Include/pds/SHA256.h
		./Include/pds/SHA256.inl
		./Include/pds/ValueTypes.h
		./Include/pds/ValueTypes.inl
		./Include/pds/Varying.h
		./Include/pds/Varying.inl
		)	
# 
	target_include_directories(	
		systemtest 
		PUBLIC ${PROJECT_SOURCE_DIR}/Include
		PUBLIC ${glm_SOURCE_DIR}
		PUBLIC ${librocksha256_SOURCE_DIR}
		PUBLIC ${ctle_SOURCE_DIR}
		PUBLIC ${PROJECT_SOURCE_DIR}/Tests 
		) 
#
	target_link_libraries( 	
		systemtest 
		)

	# Tests run unit tests and other specific tests
	add_executable( 
		tests
		./Tests/Tests.cpp 
		./Tests/DirectedGraphTests.cpp
		./Tests/DynamicTypesTests.cpp
		./Tests/EntityReaderRandomTests.cpp
		./Tests/EntityReadWriteTests.cpp
		./Tests/EntityTests.cpp
		./Tests/ItemTableTests.cpp
		./Tests/ReadWriteTests.cpp
		./Tests/SectionHierarchyReadWriteTests.cpp
		./Tests/TypeTests.cpp 
		./Tests/TestHelpers/random_vals.cpp 
		./Tests/TestPackA/TestPackA.cpp 
		)

	target_include_directories( 
		tests 
		PUBLIC ${PROJECT_SOURCE_DIR}/Include
		PUBLIC ${glm_SOURCE_DIR}
		PUBLIC ${librocksha256_SOURCE_DIR}
		PUBLIC ${ctle_SOURCE_DIR}
		PUBLIC ${PROJECT_SOURCE_DIR}/Tests	
		)

	if(MSVC)								
		target_link_libraries( 	tests 
								gtest_main 
								Rpcrt4
								)
	else()
		target_link_libraries( 	tests 
								gtest_main 
								)
	endif()

# enable_testing()	
endif()
