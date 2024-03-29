# Quick CMake recipe for GCC and MSVC.
# TODO: This used to use local libraries, but since I'm dredging this
# project up after a couple of years of inactivity, I'm having it look
# for the libraries on the host system instead. The following
# libraries must be installed and detectable by CMake:
# * AssImp
# * Bullet
# * GLFW
# * GLEW
cmake_minimum_required (VERSION 3.2)
project (Berilia_F)
set (Berilia_F_VERSION_MAJOR 0)
set (Berilia_F_VERSION_MINOR 1)

set (SOURCE_FILES src/game.cpp src/util.cpp src/shaders.cpp src/script.cpp src/gui.cpp src/lighting.cpp src/unity.cpp src/camera.cpp src/mesh.cpp src/texture.cpp src/physics.cpp src/math3d.cpp src/math2d.cpp)

# GLFW
if (MSVC)
	# (And GLEW for MSVC)
	#set (GLFW_SD "lib/GLFW_MSVC")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
	set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
	#set (GLFW_LIBRARIES "${PROJECT_SOURCE_DIR}/${GLFW_SD}/lib/glfw3.lib")
	#set (GLFW_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/GLFW_MSVC/include")
	#set (GLEW_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/glew_win/include")
	#set (GLEW_L "${PROJECT_SOURCE_DIR}/lib/glew_win/lib")
	#set (GLEW_LIBRARIES "${GLEW_L}/glew32.lib;${GLEW_L}/glew32mx.lib;${GLEW_L}/glew32mxs.lib;${GLEW_L}/glew32s.lib")
	#add_subdirectory(${GLFW_SD})
else ()
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14")
	#set (GLFW_SD "lib/GLFW")
	#set (GLFW_LIBRARIES "${PROJECT_SOURCE_DIR}/${GLFW_SD}/lib/libglfw3.a")
endif ()

# Assimp, Bullet, GLEW
if (WIN32)
	if (MSVC)
		#set (ASSIMP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/assimp_msvc/include")
		#set (ASSIMP_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/assimp_msvc/lib/assimp-vc130-mt.lib")
		#set (BULLET_ROOT "${PROJECT_SOURCE_DIR}/lib/bullet_msvc")
	else ()
		#set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR}/lib/glew_win")
		#set (CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH};${PROJECT_SOURCE_DIR}/lib/glew_win/lib")
		#set (ASSIMP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/assimp_mingw/include")
		#set (ASSIMP_LIBS_DIR "${PROJECT_SOURCE_DIR}/lib/assimp_mingw/lib")
		#set (ASSIMP_LIBRARIES "${ASSIMP_LIBS_DIR}/libassimp.dll.a;${ASSIMP_LIBS_DIR}/libzlibstatic.a;${ASSIMP_LIBS_DIR}/libassimp.dll")
		#set (BULLET_ROOT "${PROJECT_SOURCE_DIR}/lib/bullet_mingw")
	endif ()
else ()
	#set (BULLET_ROOT "${PROJECT_SOURCE_DIR}/lib/bullet/build")
endif ()

find_package (OpenGL REQUIRED)
if (NOT (WIN32 AND MSVC))
	find_package (GLEW REQUIRED)
endif ()
find_package (Bullet REQUIRED)

if (WIN32)
	if (NOT MSVC)
		#add_subdirectory("lib/GLFW")
		#set (GLFW_INCLUDE_DIRS "lib/GLFW/include")
	endif ()
else ()
	find_package (PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
	find_package (assimp REQUIRED)
endif ()

add_executable (main src/main.cpp ${SOURCE_FILES})

include_directories (${BULLET_INCLUDE_DIRS})
include_directories (${OPENGL_INCLUDE_DIRS})
include_directories (${GLEW_INCLUDE_DIRS})
include_directories (${GLFW_INCLUDE_DIRS})
include_directories (${ASSIMP_INCLUDE_DIRS})
include_directories ("inc")

if (NOT WIN32)
	target_link_libraries (main ${GLFW_STATIC_LIBRARIES};${ASSIMP_LIBRARIES};${BULLET_STATIC_LIBRARIES})
else ()
	if (MSVC)
		target_link_libraries (main "legacy_stdio_definitions.lib")
	endif ()
	target_link_libraries (main ${GLFW_LIBRARIES};${ASSIMP_LIBRARIES})
endif ()
target_link_libraries (main ${OPENGL_LIBRARIES};${GLEW_LIBRARIES};${BULLET_LIBRARIES})
