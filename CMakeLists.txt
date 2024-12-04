cmake_minimum_required(VERSION 3.20)
project("WebFrame++" VERSION 1.0.0.0 LANGUAGES CXX)

option(TEMPLATE_ENGINE "Template engine to be used." OFF)
option(WITH_TESTS "Compile unit and integration tests targets" ON)
option(WITH_BENCHMARKS "Compile benchmark targets" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMKAE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DOC "Enables/Disables the documentation target" ON)

if(DOC)
    find_package(Doxygen REQUIRED)

    # Define the Doxygen configuration file
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Configure Doxygen
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})

    # Add a custom target for generating documentation
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"

        # VERBATIM
    )

    # Add a custom command to generate the PDF
    add_custom_command(TARGET doc POST_BUILD
        COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/docs/latex
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/latex
        COMMENT "Building PDF documentation from LaTeX"
    )
endif()