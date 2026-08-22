if (NOT DEFINED TARGET_INCLUDE_DIRS OR TARGET_INCLUDE_DIRS STREQUAL "")
	return()
endif()

foreach(dir IN LISTS TARGET_INCLUDE_DIRS)
	if (EXISTS "${dir}")
		file(COPY "${dir}/" DESTINATION "${DEST_DIR}")
	endif()
endforeach()
