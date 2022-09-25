cmake_minimum_required(VERSION 3.21)

project( persistent-ds )
			
add_executable( SystemTest 
				./Tests/SystemTest.cpp 
				)			
						
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(SystemTest PUBLIC
                          "${PROJECT_SOURCE_DIR}/Include"

						  # dependencies
						  "${PROJECT_SOURCE_DIR}/Dependencies/ctle"
						  "${PROJECT_SOURCE_DIR}/Dependencies/fast_float/include"
						  "${PROJECT_SOURCE_DIR}/Dependencies/glm"
						  "${PROJECT_SOURCE_DIR}/Dependencies/librock_sha256"
                          )
									
target_link_libraries( SystemTest PUBLIC
					   Rpcrt4
					   )
