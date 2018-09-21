".\bin\win_x64\dxc.exe" -spirv -T vs_6_0 -E ColorVertexShader color.hlsl -Fo color.spv
".\bin\win_x64\SPIRV-Cross.exe" --version 150 color.spv --output color.glsl
".\bin\win_x64\SPIRV-Cross.exe" --version 310 --es color.spv --output color.gles
".\bin\win_x64\SPIRV-Cross.exe" --msl color.spv --output color.metal
pause