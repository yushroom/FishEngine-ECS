from glob import glob
import os, sys

bin_dir = "../../Tools/ShaderCompiler/bin"
if sys.platform == 'win32':
	bin_dir = os.path.join(bin_dir, "win_x64")
	bin_dir = os.path.normpath(os.path.abspath(bin_dir))
	dxc = os.path.join(bin_dir, "dxc.exe")
	spvir_cross = os.path.join(bin_dir, "SPIRV-Cross.exe")
elif sys.platform == 'darwin':
	bin_dir = os.path.join(bin_dir, "osx")
	bin_dir = os.path.normpath(os.path.abspath(bin_dir))
	# dxc = os.path.join(bin_dir, "dxc-3.7")
	dxc = "/Users/yushroom/program/github/DirectXShaderCompiler/build/bin/dxc-3.7"
	spvir_cross = os.path.join(bin_dir, "spirv-cross")
elif eys.platform == 'linux':
	exit()

shader_include_dir = './include'
# shader_include_dir = '/Users/yushroom/program/UnrealEngine/Engine/Shaders/Private'

assert(os.path.exists(dxc))
assert(os.path.exists(spvir_cross))

# os.system(dxc + " --help")
# exit()

def RunCommand(cmd):
	print(cmd)
	ret = os.system(cmd)
	assert(ret == 0)

def compile(hlsl_path):
	fn = os.path.splitext(os.path.basename(hlsl_path))[0]

	for type in ('vs', 'ps'):
		glsl_path = fn + f'_{type}.glsl'
		metal_path = fn + f'_{type}.metal'
		spv = f"{type}.spv"
		glsl_path = os.path.join('runtime', glsl_path)
		metal_path = os.path.join('runtime', metal_path)
		spv = os.path.join('runtime', spv)

		entry_point = type.upper()
		# cmd = f"{dxc} {hlsl_path}"
		# RunCommand(cmd)
		# -spirv
		cmd1 = f"{dxc} -spirv -T {type}_6_0 -E {entry_point} {hlsl_path} -I {shader_include_dir} -Fo {spv}"
		RunCommand(cmd1)
		cmd2 = f"{spvir_cross} --version 410 {spv} --output {glsl_path}"
		cmd3 = f"{spvir_cross} --msl {spv} --output {metal_path}"
		RunCommand(cmd2)
		RunCommand(cmd3)

for f in glob("*.hlsl"):
	print(f)
	compile(f)