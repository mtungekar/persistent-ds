cmake_minimum_required(VERSION 3.22)
project( persistent-ds VERSION 0.2 LANGUAGES CXX )

include(FetchContent)

# vcpkg 
FetchContent_Declare(
	vcpkg
	GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
	GIT_TAG        2023.01.09 #f14984af3738e69f197bf0e647a8dca12de92996 # 2023.01.09
    SOURCE_DIR vcpkg
)

FetchContent_MakeAvailable(vcpkg)

FetchContent_GetProperties(vcpkg)

# bootstrap vcpkg and install required packages
if(NOT vcpkg_POPULATED)
    message(FATAL_ERROR "Failed to fetch  vcpkg. Aborting....")
else()
    if(WIN32)
        message(STATUS "Run bootstrap-vcpkg")
        execute_process(COMMAND cmd /C "${vcpkg_SOURCE_DIR}/bootstrap-vcpkg.bat" COMMAND_ECHO STDOUT )
        message(STATUS "Installing x64-windows triplet")
        execute_process(COMMAND cmd /C "${vcpkg_SOURCE_DIR}/vcpkg.exe install --triplet x64-windows" COMMAND_ECHO STDOUT)
    elseif(LINUX)
        execute_process(COMMAND cmd /C "${vcpkg_SOURCE_DIR}/bootstrap-vcpkg.sh" COMMAND_ECHO STDOUT)
        message(STATUS "Installing x64-linux triplet")
        execute_process(COMMAND cmd /C "${vcpkg_SOURCE_DIR}/vcpkg install --triplet x64-linux" COMMAND_ECHO STDOUT)
    else()
        message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} not supported currently. Aborting....")
    endif()
    # include toolchain file. This is usually setup through command line if vcpkg is a submodule or already checked out in anoter location.
	message(STATUS "${CMAKE_TOOLCHAIN_FILE} ")
	include(${CMAKE_TOOLCHAIN_FILE})
    
    # enable vcpkg manifest mode
    set(ENV{VCPKG_FEATURE_FLAGS} "manifests")
endif()

# options can be setup in CMakePresets.json/CMkaeUserPrests.json (cacheVariables). Or passed through the command lines.
option(ENABLE_UNIT_TESTING "Enable Test Builds" OFF)
option(ENABLE_CLANG_FORMAT "Enable clang format for generated code." OFF)

include(${CMAKE_SOURCE_DIR}/cmake/project_settings.cmake)
include (${CMAKE_SOURCE_DIR}/cmake/compiler_warnings.cmake)

set_project_warnings(${PROJECT_NAME})

# find python
find_package(PythonInterp REQUIRED)
find_package(Python REQUIRED)

# check if code generation should format using clang
if(ENABLE_CLANG_FORMAT)
	set(SCRIPT_ARGS "--clang_format")
endif()

# run code generator
execute_process( 
	COMMAND 			${PYTHON_EXECUTABLE} CodeGenerator.py ${SCRIPT_ARGS}
	WORKING_DIRECTORY 	${CMAKE_CURRENT_LIST_DIR}/CodeGen
	RESULT_VARIABLE 	py_result
)
message(STATUS "Result of CodeGenerator.py: ${py_result}")

	
# if this is the main build of persistent-ds, build the testing code
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)			
				
	# generate the pds test code
	execute_process( 
		COMMAND 			${PYTHON_EXECUTABLE} GenerateTestPacks.py ${SCRIPT_ARGS}
		WORKING_DIRECTORY 	${CMAKE_CURRENT_LIST_DIR}/CodeGen
		RESULT_VARIABLE 	py_result
	)
	message(STATUS "Result of GenerateTestPacks.py: ${py_result}")
	
	# add include subdirectory (has its own list files with install setup)
	add_subdirectory(Include)

	# if unit testing is enabeld add Test Directory
	if(ENABLE_UNIT_TESTING)
  		enable_testing()
		add_subdirectory(Tests)
	endif()
endif()
