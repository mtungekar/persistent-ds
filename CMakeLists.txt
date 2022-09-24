cmake_minimum_required(VERSION 3.21)

project( persistent-ds )

add_library(persistent-ds 
			./Src/pds.cpp 
			./Src/Varying.cpp 
			./Src/SHA256.cpp 
			./Src/DataTypes.cpp 
			./Src/DynamicTypes.cpp 
			./Src/EntityReader.cpp 
			./Src/EntityWriter.cpp 
			./Src/ValueTypes.cpp 
			)
			
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(persistent-ds PUBLIC
                          "${PROJECT_SOURCE_DIR}/Include"

						  # dependencies
						  "${PROJECT_SOURCE_DIR}/Dependencies/ctle"
						  "${PROJECT_SOURCE_DIR}/Dependencies/fast_float/include"
						  "${PROJECT_SOURCE_DIR}/Dependencies/glm"
						  "${PROJECT_SOURCE_DIR}/Dependencies/librock_sha256"
                          )
			
add_executable( SystemTest 
				./Tests/SystemTest.cpp 
				)			
							
target_link_libraries( SystemTest PUBLIC
					   persistent-ds
					   )
				