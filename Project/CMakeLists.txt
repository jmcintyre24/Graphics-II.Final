cmake_minimum_required(VERSION 3.16)

project(DEV4_FinalWObjLoader)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_executable (FinalWObjLoader main.cpp DDSTextureLoader.cpp DDSTextureLoader.h)
target_link_libraries(FinalWObjLoader d3d11.lib d3dcompiler.lib)

file(COPY ".\\Models\\balloon.obj" DESTINATION Models)
file(COPY ".\\Models\\crossbow.obj" DESTINATION Models)
file(COPY ".\\Textures\\LongMattedGrass.dds" DESTINATION Textures)
file(COPY ".\\Textures\\lowpoly_crossbow.dds" DESTINATION Textures)
file(COPY ".\\Textures\\LostValley.dds" DESTINATION Textures)
file(COPY ".\\Textures\\crosshair.dds" DESTINATION Textures)
file(COPY ".\\Shaders\\shaders.fx" DESTINATION Shaders)
file(COPY ".\\Shaders\\DEV4_PS.hlsl" DESTINATION Shaders)
file(COPY ".\\Shaders\\DEV4_GS.hlsl" DESTINATION Shaders)
file(COPY ".\\Shaders\\DEV4_VS.hlsl" DESTINATION Shaders)