

# packages used by the executables
find_package(glm CONFIG REQUIRED)
find_package(ctle CONFIG REQUIRED)
find_path(PICOSHA2_INCLUDE_DIRS "picosha2.h")


set(FUZZING_TEST_EXECUTABLE "fuzzingtests")
# tests run unit tests and other specific tests
add_executable(${FUZZING_TEST_EXECUTABLE})

# tests target srouces
target_sources(${FUZZING_TEST_EXECUTABLE}
PRIVATE
    FuzzingTest.cpp
)

# tests target include directories
target_include_directories(${FUZZING_TEST_EXECUTABLE} PRIVATE .)
target_include_directories(${FUZZING_TEST_EXECUTABLE} PRIVATE ${PICOSHA2_INCLUDE_DIRS})

# target specfic compiler options
if(MSVC)
    target_compile_options(${FUZZING_TEST_EXECUTABLE} PRIVATE /bigobj)
else()
    message(AUTHOR_WARNING "No bigobj set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()

# tests target compile options (CompilerWarnigns)
target_compile_options(${FUZZING_TEST_EXECUTABLE} PRIVATE ${COMPILER_WARNINGS})

target_compile_options(${FUZZING_TEST_EXECUTABLE} PRIVATE /fsanitize=address /fsanitize=fuzzer)

target_compile_definitions(${FUZZING_TEST_EXECUTABLE} PRIVATE CTLE_IMPLEMENTATION)


# tests target include directories
target_link_libraries(${FUZZING_TEST_EXECUTABLE}  
    PRIVATE
        pds
        glm::glm
)


