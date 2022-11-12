cmake_minimum_required(VERSION 3.21)

project( persistent-ds )

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
	include(FetchContent)
	FetchContent_Declare(
	  googletest
	  URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip
	)
	FetchContent_MakeAvailable(googletest)			
	
	# SystemTest run full system tests
	add_executable( SystemTest
					./Tests/SystemTest.cpp 
					./Tests/TestPackA/TestPackA.cpp 
					)	
	target_include_directories(	SystemTest 
								PUBLIC
									"${PROJECT_SOURCE_DIR}/Include"
									"${PROJECT_SOURCE_DIR}/Dependencies/ctle"
									"${PROJECT_SOURCE_DIR}/Dependencies/fast_float/include"
									"${PROJECT_SOURCE_DIR}/Dependencies/glm"
									"${PROJECT_SOURCE_DIR}/Dependencies/librock_sha256"
									"${PROJECT_SOURCE_DIR}/Tests"									
								)
	target_link_libraries( 	SystemTest 
							Rpcrt4
							)

	# Tests run unit tests and other specific tests
	add_executable( Tests
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
	target_include_directories( Tests 
								PUBLIC
									"${PROJECT_SOURCE_DIR}/Include"
									"${PROJECT_SOURCE_DIR}/Dependencies/ctle"
									"${PROJECT_SOURCE_DIR}/Dependencies/fast_float/include"
									"${PROJECT_SOURCE_DIR}/Dependencies/glm"
									"${PROJECT_SOURCE_DIR}/Dependencies/librock_sha256"
									"${PROJECT_SOURCE_DIR}/Tests"
								)
	target_link_libraries( 	Tests 
							gtest_main 
							Rpcrt4
							)

	# enable_testing()	
	
endif()
