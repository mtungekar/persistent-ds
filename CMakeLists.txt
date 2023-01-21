cmake_minimum_required(VERSION 3.21)

include(FetchContent)

project( persistent-ds )

# glm 
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG        bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)
FetchContent_MakeAvailable( glm )
			
# ctle 
FetchContent_Declare(
	ctle
	GIT_REPOSITORY https://github.com/Cooolrik/ctle.git
	GIT_TAG		   main
)
FetchContent_MakeAvailable( ctle )						

# librocksha256 
FetchContent_Declare(
	librocksha256
	GIT_REPOSITORY https://github.com/forrestcavalier/librock_sha256.git
	GIT_TAG		   387f4cfff7c5913bcbeaf4c1c1f53f657cc4a6d0
)
FetchContent_MakeAvailable( librocksha256 )						

# lots of warnings and all warnings as errors
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# big object files
if (MSVC)
  add_compile_options(/bigobj)
else ()
  add_compile_options(-Wa,-mbig-obj)
endif ()
			
# if this is the main build of persistent-ds, build the testing code
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)			
				
	# dowload gtest
	FetchContent_Declare(
	  googletest
	  URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip
	)
	FetchContent_MakeAvailable(googletest)			
	
	# SystemTest run full system tests
	add_executable( systemtest
					./Tests/SystemTest.cpp 
					./Tests/TestPackA/TestPackA.cpp 
					)	
	target_include_directories(	systemtest 
								PUBLIC ${PROJECT_SOURCE_DIR}/Include
								PUBLIC ${glm_SOURCE_DIR}
								PUBLIC ${librocksha256_SOURCE_DIR}/include
								PUBLIC ${ctle_SOURCE_DIR}
								PUBLIC ${Vulkan_INCLUDE_DIR}
								PUBLIC ${PROJECT_SOURCE_DIR}/Tests
								)
	target_link_libraries( 	systemtest 
							Rpcrt4
							)

	# Tests run unit tests and other specific tests
	add_executable( tests
					./Tests/DirectedGraphTests.cpp
					./Tests/DynamicTypesTests.cpp
					./Tests/EntityReaderRandomTests.cpp
					./Tests/EntityReadWriteTests.cpp
					./Tests/EntityTests.cpp
					./Tests/ItemTableTests.cpp
					./Tests/ReadWriteTests.cpp
					./Tests/SectionHierarchyReadWriteTests.cpp
					./Tests/Tests.cpp 
					./Tests/TypeTests.cpp 
					./Tests/TestHelpers/random_vals.cpp 
					./Tests/TestPackA/TestPackA.cpp 
					)
	target_include_directories( tests 
								PUBLIC ${PROJECT_SOURCE_DIR}/Include
								PUBLIC ${glm_SOURCE_DIR}
								PUBLIC ${librocksha256_SOURCE_DIR}/include
								PUBLIC ${ctle_SOURCE_DIR}
								PUBLIC ${Vulkan_INCLUDE_DIR}
								PUBLIC ${PROJECT_SOURCE_DIR}/Tests
								)
	target_link_libraries( 	tests 
							gtest_main 
							Rpcrt4
							)

# enable_testing()	
endif()
