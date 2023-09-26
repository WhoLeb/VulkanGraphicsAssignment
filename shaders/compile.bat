del .\shaders\vert.spv
del .\shaders\frag.spv

C:\VulkanSDK\1.3.239.0\Bin\glslc.exe .\shaders\shader.vert -o .\shaders\vert.spv
C:\VulkanSDK\1.3.239.0\Bin\glslc.exe .\shaders\shader.frag -o .\shaders\frag.spv

C:\VulkanSDK\1.3.239.0\Bin\glslc.exe .\shaders\spline.vert -o .\shaders\splineVert.spv
C:\VulkanSDK\1.3.239.0\Bin\glslc.exe .\shaders\spline.frag -o .\shaders\splineFrag.spv

echo "Compilation successful"