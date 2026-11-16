#[[++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Build of the LibEOS C++ examples. Built from the root CMakeLists.txt
          (option LIBEOS_BUILD_EXAMPLES), standalone in the repository with
          -DLIBEOS_DLL=<path to lib3mftoolpath4eos.dll>, or standalone in the examples/ folder of a binary release.

]]

cmake_minimum_required(VERSION 3.10)

set(LIBEOS_EXAMPLES_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
if (EXISTS ${LIBEOS_EXAMPLES_PARENT_DIR}/include/LibEOS/CppDynamic)
	# Binary release layout
	set(LIBEOS_EXAMPLES_INCLUDE_DIRS
		${LIBEOS_EXAMPLES_PARENT_DIR}/include/LibEOS/CppDynamic
		${LIBEOS_EXAMPLES_PARENT_DIR}/include/Lib3MF/CppDynamic
	)
	set(LIBEOS_EXAMPLES_LIB3MF_DLL ${LIBEOS_EXAMPLES_PARENT_DIR}/bin/lib3mf.dll)
	set(LIBEOS_EXAMPLES_DEFAULT_LIBEOS_DLL ${LIBEOS_EXAMPLES_PARENT_DIR}/bin/lib3mftoolpath4eos.dll)
else()
	# Repository layout
	set(LIBEOS_EXAMPLES_INCLUDE_DIRS
		${LIBEOS_EXAMPLES_PARENT_DIR}/Bindings/CppDynamic
		${LIBEOS_EXAMPLES_PARENT_DIR}/Libraries/lib3mf/bin/Bindings/CppDynamic
	)
	set(LIBEOS_EXAMPLES_LIB3MF_DLL ${LIBEOS_EXAMPLES_PARENT_DIR}/Libraries/lib3mf/bin/lib3mf.dll)
	set(LIBEOS_EXAMPLES_DEFAULT_LIBEOS_DLL "")
endif()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	project(LibEOSExamples CXX)
	set(CMAKE_CXX_STANDARD 17)
	set(LIBEOS_DLL "${LIBEOS_EXAMPLES_DEFAULT_LIBEOS_DLL}" CACHE FILEPATH "Built lib3mftoolpath4eos.dll to copy next to the examples")
endif()

set(LIBEOS_EXAMPLE_NAMES
	00_generate_sample_3mf
	01_convert_3mf_openjz
	02_openjz_to_machine
	03_mesh3mf_to_machine
	06_toolpath3mf_to_machine
	toolpath_credential
)

foreach(EXAMPLE_NAME ${LIBEOS_EXAMPLE_NAMES})
	add_executable(${EXAMPLE_NAME} cpp/${EXAMPLE_NAME}.cpp cpp/example_common.hpp)
	target_include_directories(${EXAMPLE_NAME} PRIVATE ${LIBEOS_EXAMPLES_INCLUDE_DIRS})
	if (MSVC)
		target_compile_definitions(${EXAMPLE_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
	endif()
	if (WIN32)
		target_link_libraries(${EXAMPLE_NAME} PRIVATE Advapi32)
	endif()
	set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER Examples)

	# The examples load lib3mftoolpath4eos.dll and lib3mf.dll from their own directory.
	add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBEOS_EXAMPLES_LIB3MF_DLL} $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)

	if (TARGET libeos)
		add_dependencies(${EXAMPLE_NAME} libeos)
		add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:libeos> $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)
	elseif (LIBEOS_DLL)
		add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBEOS_DLL} $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)
	endif()
endforeach()

if (TARGET libeos)
	install(TARGETS ${LIBEOS_EXAMPLE_NAMES} RUNTIME DESTINATION bin)
	install(FILES CMakeLists.txt README.md DESTINATION examples)
	install(DIRECTORY cpp DESTINATION examples)
endif()
