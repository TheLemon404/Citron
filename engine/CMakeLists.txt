set(CITRON_SDK_DIR ${CMAKE_BINARY_DIR}/editor/sdk)

function(citron_export_sdk_headers target)
	if (NOT TARGET ${target})
		message(WARNING "citron_export_sdk_headers: '${target}' is not a target, skipping")
		return()
	endif()

	add_custom_command(TARGET ${target} POST_BUILD
								COMMAND ${CMAKE_COMMAND}
												-DTARGET_INCLUDE_DIRS=$<TARGET_PROPERTY:${target},INTERFACE_INCLUDE_DIRECTORIES>
												-DDEST_DIR=${CITRON_SDK_DIR}/include/${target}
												-P "${CMAKE_SOURCE_DIR}/cmake/CopySdkHeaders.cmake"
								COMMENT "Exporting public headers for ${target}"
								VERBATIM
				)
endfunction()


add_subdirectory(assets)
#add_subdirectory(audio)
add_subdirectory(core)
add_subdirectory(ecs)
add_subdirectory(graphics)
add_subdirectory(input)
add_subdirectory(io)
#add_subdirectory(networking)
#add_subdirectory(physics)
add_subdirectory(util)
add_subdirectory(scripting)
add_subdirectory(app)
