function(group_source src_list file_prefix group_prefix) # group src files in vs filters as how they look like in filesystem
	if (WIN32)
		foreach(FILE ${src_list}) 
			get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
			string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}${file_prefix}" "" GROUP "${PARENT_DIR}")
			string(REPLACE "/" "\\" GROUP "${GROUP}")
			source_group("${group_prefix}${GROUP}" FILES "${FILE}")
		endforeach()
	endif()
endfunction()

macro(set_output_dir dir)
	foreach(c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${c} c )
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${c} ${dir})
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${c} ${dir})
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${c} ${dir})
	endforeach()
endmacro()
