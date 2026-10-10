# Sandbox

set(SOURCE_DIR "src")

set(SOURCES
	src/SandboxApp.cpp
)

add_executable(SandboxApp ${SOURCES})

target_link_libraries(SandboxApp Radiant)