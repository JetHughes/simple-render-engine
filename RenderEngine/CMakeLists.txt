# CMake entry point
cmake_minimum_required (VERSION 3.1)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/Jet Hughes/vcpkg/scripts/buildsystems/vcpkg.cmake")
project (COSC_RenderEngine)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies
add_subdirectory (external)


if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)



include_directories(
	${GLM_INCLUDE_DIRS}
	${GLFW_INCLUDE_DIRS}
	external/assimp-3.0.1270/include/
	external/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	${GLEW_LIBRARIES}

)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)



# OpenGL Skeleton
add_executable(Skeleton
Skeleton/renderApp.cpp
common/Shader.cpp
common/Shader.hpp
common/Texture.cpp
common/Texture.hpp
common/Texture.cpp
common/Texture.hpp
common/Camera.hpp
common/Camera.cpp
common/Scene.hpp
common/Scene.cpp
common/Object.hpp
common/Object.cpp
common/MTLShader.hpp
common/MTLShader.cpp
common/Mesh.hpp
common/Mesh.cpp
common/Material.hpp
common/Material.cpp
common/Group.hpp
common/Group.cpp
common/Objloader.hpp
common/Objloader.cpp
common/Controls.hpp
common/Controls.cpp
common/Quad.cpp
common/Quad.hpp
common/PostProcessingShader.hpp
common/PostProcessingShader.cpp

Skeleton/mtlShader.frag
Skeleton/mtlShader.vert
Skeleton/Passthrough.vert
Skeleton/PostEffect.frag
)
target_link_libraries(Skeleton
${ALL_LIBS}
assimp
)

set_target_properties(Skeleton PROPERTIES COMPILE_DEFINITIONS "USE_ASSIMP")
set_target_properties(Skeleton PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Skeleton/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )

if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET Skeleton POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/Skeleton${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/Skeleton/"
)




elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
