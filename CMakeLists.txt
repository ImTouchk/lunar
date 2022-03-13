cmake_minimum_required(VERSION 3.21)
project(lunar)

set(LUNAR_BUNDLE_SAMPLE_ASSETS ON)

add_subdirectory(engine)
add_subdirectory(asset-packer)
#add_subdirectory(lunarsl)

add_custom_target("lunar-assets")

add_custom_command(
	TARGET "lunar-assets" POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets/compiled"
)

if(LUNAR_RENDERER_BACKEND STREQUAL "Vulkan")
	list(APPEND VULKAN_SHADERS "shader.vert" "shader.frag")

	foreach(shader ${VULKAN_SHADERS})
		add_custom_command(
			TARGET "lunar-assets" POST_BUILD
			COMMAND ${glslc} "${CMAKE_CURRENT_SOURCE_DIR}/assets/${shader}" -o "${CMAKE_CURRENT_SOURCE_DIR}/assets/compiled/${shader}"
			DEPENDS ${shader} "assets/compiled"
		)
	endforeach()
endif()

add_custom_command(
	TARGET "lunar-assets" POST_BUILD
	COMMAND $<TARGET_FILE:asset-packer> "${CMAKE_CURRENT_SOURCE_DIR}/assets/compiled" "${CMAKE_CURRENT_SOURCE_DIR}/assets/default.vpak" "Default"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/default.vpak" "${LUNAR_BINARY_DIR}/default.vpak"
	COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_SOURCE_DIR}/assets/default.vpak"
	DEPENDS "default.vpak" "${LUNAR_BINARY_DIR}"
)

if(LUNAR_BUNDLE_SAMPLE_ASSETS)
	file(DOWNLOAD "https://cdn.pixabay.com/photo/2016/03/23/19/37/statue-1275469_960_720.jpg" "assets/statue.jpg")
endif()

add_dependencies("lunar" "lunar-assets")


