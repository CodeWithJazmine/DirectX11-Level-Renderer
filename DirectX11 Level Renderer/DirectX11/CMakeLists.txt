cmake_minimum_required(VERSION 3.16)

project(D3D11_Level_Renderer)

# CMake FXC shader compilation, add any shaders you want compiled here
set(VERTEX_SHADERS 
	# add vertex shader (.hlsl) files here
	Shaders/VertexShader.hlsl
)

set(PIXEL_SHADERS 
	# add pixel shader (.hlsl) files here
	Shaders/PixelShader.hlsl
)

# Add any new C/C++ source code here
set(SOURCE_CODE
	# Header & CPP files go here
	Source/main.cpp
	Source/RenderManager.h
	Source/Utility/FileIntoString.h
	Source/Utility/h2bParser.h
	Source/Utility/load_object_oriented.h
	Source/Camera.h
	Source/DialogBox.h
	Source/Audio.h
)

if(WIN32)
# by default CMake selects "ALL_BUILD" as the startup project 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY VS_STARTUP_PROJECT D3D11_Level_Renderer)

	#this will tell Visual Studio to put the shaders under their own group in the solution explorer 
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})	
endif()

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)



add_executable (D3D11_Level_Renderer 
	${SOURCE_CODE}
	${VERTEX_SHADERS}
	${PIXEL_SHADERS}
)

set_source_files_properties( ${VERTEX_SHADERS} PROPERTIES 
        VS_SHADER_TYPE Vertex 
        VS_SHADER_MODEL 5.0
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "FXCompile" 
)
set_source_files_properties( ${PIXEL_SHADERS} PROPERTIES 
	    VS_SHADER_TYPE Pixel 
        VS_SHADER_MODEL 5.0
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "FXCompile"
)