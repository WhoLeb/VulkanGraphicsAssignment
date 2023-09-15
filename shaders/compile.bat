del .\shaders\vert.spv
del .\shaders\frag.spv

C:\VulkanSDK\1.3.250.0\Bin\glslc.exe .\shaders\shader.vert -o .\shaders\vert.spv
C:\VulkanSDK\1.3.250.0\Bin\glslc.exe .\shaders\shader.frag -o .\shaders\frag.spv

echo "Compilation successful"