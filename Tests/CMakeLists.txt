
# create test folder for systemtest.cpp
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestFolder)

# packages used by the executables
find_package(glm CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(ctle CONFIG REQUIRED)
find_path(PICOSHA2_INCLUDE_DIRS "picosha2.h")

include(GoogleTest)

# setup executbale names
set(SYSTEMTEST_EXECUTABLE "systemtest")

# systemtest run full system tests
add_executable(${SYSTEMTEST_EXECUTABLE})

# systemtest target soruces
target_sources(${SYSTEMTEST_EXECUTABLE}
    PRIVATE
        SystemTest.cpp
        TestPackA/TestPackA.cpp
    )

# systemtest include directories
target_include_directories(${SYSTEMTEST_EXECUTABLE} PRIVATE .)
target_include_directories(${SYSTEMTEST_EXECUTABLE} PRIVATE TestHelpers)
target_include_directories(${SYSTEMTEST_EXECUTABLE} PRIVATE TestPackA)
target_include_directories(${SYSTEMTEST_EXECUTABLE} PRIVATE ${PICOSHA2_INCLUDE_DIRS})

# systemtest setup compiler warnings
target_compile_options(${SYSTEMTEST_EXECUTABLE} PRIVATE ${COMPILER_WARNINGS})

# systemtest link packages
target_link_libraries(${SYSTEMTEST_EXECUTABLE} 
    PRIVATE 
        GTest::gtest
        GTest::gtest_main  
        pds 
        glm::glm
)

# Discover test
gtest_discover_tests(${SYSTEMTEST_EXECUTABLE})

############################################################################

set(TESTS_EXECUTABLE "tests")
# tests run unit tests and other specific tests
add_executable(${TESTS_EXECUTABLE})

# tests target srouces
target_sources(${TESTS_EXECUTABLE}
PRIVATE
    DirectedGraphTests.cpp
    DynamicTypesTests.cpp
    EntityReaderRandomTests.cpp
    EntityReadWriteTests.cpp
    EntityTests.cpp
    ItemTableTests.cpp
    ReadWriteTests.cpp
    SectionHierarchyReadWriteTests.cpp
    Tests.cpp 
    TypeTests.cpp 
    TestHelpers/random_vals.cpp 
    TestPackA/TestPackA.cpp 
)

# tests target include directories
target_include_directories(${TESTS_EXECUTABLE} PRIVATE .)
target_include_directories(${TESTS_EXECUTABLE} PRIVATE TestHelpers)
target_include_directories(${TESTS_EXECUTABLE} PRIVATE TestPackA)
target_include_directories(${TESTS_EXECUTABLE} PRIVATE ${PICOSHA2_INCLUDE_DIRS})

# target specfic compiler options
if(MSVC)
    target_compile_options(${TESTS_EXECUTABLE} PRIVATE /bigobj)
else()
    message(AUTHOR_WARNING "No bigobj set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()

# tests target compile options (CompilerWarnigns)
target_compile_options(${TESTS_EXECUTABLE} PRIVATE ${COMPILER_WARNINGS})



# tests target include directories
target_link_libraries(${TESTS_EXECUTABLE}  
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        pds
        glm::glm
)


gtest_discover_tests(${TESTS_EXECUTABLE})