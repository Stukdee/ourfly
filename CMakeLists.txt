cmake_minimum_required(VERSION 3.11)

#项目名称为ourfly
project(ourfly)

#要求使用c++23
set(CMAKE_CXX_STANDARD 23)

#强制使用项目的raylib库
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#OpenGL ES 支持（使用 Wayland 模式）
if (UNIX AND NOT APPLE)
	set(PLATFORM "Desktop" CACHE STRING "" FORCE)
	set(OPENGL_VERSION "ES 2.0" CACHE STRING "" FORCE)
	set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
	set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
endif()

#添加raylib
add_subdirectory(raylib)

#素材文件
file(GLOB_RECURSE RESOURCE_FILES
	assets
)

#添加源码
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

#修改这里：移除Info.plist依赖
set_target_properties(${PROJECT_NAME} PROPERTIES
	MACOSX_BUNDLE FALSE
)

# 修改资源复制路径
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
	$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/assets
	$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# macOS 平台特定链接
if (APPLE)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		"-framework IOKit"
		"-framework Cocoa"
		"-framework OpenGL"
	)
endif()

# Linux 平台特定链接（Wayland 模式）
if (UNIX AND NOT APPLE)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		GLESv2
		EGL
		m
		pthread
		dl
	)
endif()