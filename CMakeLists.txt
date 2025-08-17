cmake_minimum_required(VERSION 3.14)
project(docparser VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(pybind11 REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create the Python module
pybind11_add_module(docparser 
    python_bindings.cpp
)

# Compiler-specific options
target_compile_definitions(docparser PRIVATE VERSION_INFO="${PROJECT_VERSION}")

# Set properties for the module
set_target_properties(docparser PROPERTIES
    CXX_VISIBILITY_PRESET "hidden"
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(docparser PRIVATE _WIN32_WINNT=0x0601)
endif()

# Optional: Build standalone C++ library as well
option(BUILD_CPP_LIBRARY "Build standalone C++ library" ON)

if(BUILD_CPP_LIBRARY)
    # Create header-only library target for C++ users
    add_library(docparser_cpp INTERFACE)
    target_include_directories(docparser_cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_features(docparser_cpp INTERFACE cxx_std_17)
    
    # Install headers for C++ library
    install(FILES document_parser.h 
            DESTINATION include/docparser)
endif()

# Optional: Build tests
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(test_docparser
        tests/test_document_parser.cpp
    )
    
    target_link_libraries(test_docparser
        GTest::gtest_main
        docparser_cpp
    )
    
    include(GoogleTest)
    gtest_discover_tests(test_docparser)
endif()

# Installation rules
install(TARGETS docparser
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "docparser")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Universal Document Parser Library")
set(CPACK_PACKAGE_VENDOR "Document Parser Team")
set(CPACK_PACKAGE_CONTACT "dev@example.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)