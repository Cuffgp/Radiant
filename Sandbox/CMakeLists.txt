# Sandbox

set(SOURCE_DIR "src")

set(SOURCES
	src/SandboxApp.cpp
)

add_executable(SandboxApp ${SOURCES})

target_link_libraries(SandboxApp Radiant)

set_property(TARGET SandboxApp PROPERTY 
	DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)