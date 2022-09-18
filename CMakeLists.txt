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
                          "${PROJECT_SOURCE_DIR}/include"
						  "${PROJECT_SOURCE_DIR}/Dependencies"
                          )
			