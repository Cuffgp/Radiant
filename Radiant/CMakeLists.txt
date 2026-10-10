# Radiant

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw-3.4)

# Volk
set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
add_subdirectory(vendor/volk-1.4.350)

add_subdirectory(vendor/VulkanMemoryAllocator-3.4.0)


# Library
add_library(Radiant)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h")

target_sources(Radiant
	PRIVATE
		${SOURCES}
	PUBLIC
		${HEADERS}
)

#target_link_directories(Radiant
#	PUBLIC
#)

find_library(SLANG 
    NAMES slang
    HINTS "vendor/slang-2026.19/lib"
)

target_link_libraries(Radiant
	PRIVATE
	glfw
	volk
)

target_include_directories(Radiant
	PUBLIC
		src
		vendor/vulkan-headers-1.4.354
		vendor/glfw-3.4/include
		vendor/volk-1.4.350
		vendor/VulkanMemoryAllocator-3.4.0/include
)