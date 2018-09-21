from glob import glob
import os

bin_dir = "../../Tools/ShaderCompiler/bin/win_x64"
dxc = os.path.join(bin_dir, "dxc.exe")
spvir_cross = os.path.join(bin_dir, "SPIRV-Cross.exe")

assert(os.path.exists(dxc))
assert(os.path.exists(spvir_cross))

def compile(hlsl_path):
	glsl_path = 'test.vs'
	cmd1 = f"{dxc} -spirv -T vs_6_0 -E VS {hlsl_path} -Fo temp.spv"
	cmd2 = f"{spvir_cross} --version 410 temp.spv --output {glsl_path}"
	cmd1 = os.path.normpath(cmd1)
	cmd2 = os.path.normpath(cmd2)
	print(cmd1)
	ret = os.system(cmd1)
	assert(ret == 0)
	print(cmd2)
	ret = os.system(cmd2)
	assert(ret == 0)
	# "./bin/win_x64/SPIRV-Cross.exe --version 310 --es color.spv --output color.gles"
	# "./bin/win_x64/SPIRV-Cross.exe --msl color.spv --output color.metal"

for f in glob("*.hlsl"):
	compile(f)