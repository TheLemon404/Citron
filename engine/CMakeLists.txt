set(CITRON_SDK_DIR ${CMAKE_BINARY_DIR}/editor/sdk)

function(citron_export_sdk_headers target)
	add_custom_command(TARGET ${target} POST_BUILD
								COMMAND ${CMAKE_COMMAND} -E copy_directory
												$<TARGET_PROPERTY:${target},SOURCE_DIR>/include
												${CITRON_SDK_DIR}/include/${target}
								COMMENT "Exporting public headers for ${target}"
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
