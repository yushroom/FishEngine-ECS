from glob import glob
import os, sys
from enum import Enum

class ShaderType(Enum):
	OpenGL = 0
	Metal = 1
	D3D12 = 2

render_type = ShaderType.D3D12

shader_src_dir = "./Shaders"
out_dir = './Shaders/runtime'
if sys.platform == 'darwin':
	shaderc = "./ThirdParty/bgfx/.build/osx64_clang/bin/shadercDebug"
	platform = 'osx'
else:
	shaderc = "./ThirdParty/bgfx/.build/win64_vs2017/bin/shadercDebug.exe"
	platform = 'windows'

include_paths = ["./ThirdParty/bgfx/src", "./Shaders/include"]

headers = ' '.join(['-i ' + x for x in include_paths])

# profile = None
# # platform = 'windows'
# # profile = 'vs_4_0'
# # platform = 'osx'
# profile = 'metal'
# # profile = 210
# platform = 'linux'
# profile = '150'

if not os.path.exists(out_dir):
	os.mkdir(out_dir)

for shader_file in glob(os.path.join(shader_src_dir, "*.shader")):
	print(f'compile {shader_file}...')
	fn = os.path.split(shader_file)[-1]
	bn = fn.rsplit('.', 1)[0]
	if bn == 'PBR':
		continue
	for ext, shader_type, define in [('_vs.bin', 'vertex', 'VERTEX'), ('_fs.bin', 'fragment', 'FRAGMENT')]:
		compiled_shader_file = os.path.join(out_dir, bn + ext)
		cmd = f"{shaderc} {headers} --verbose -f {shader_file} -o {compiled_shader_file} --platform {platform} --type {shader_type} --define {define}"
		cmd += " --varyingdef ./Shaders/varying.def.sc"
		# if profile:
		# 	cmd += f' --profile {profile}'
		if render_type == ShaderType.D3D12:
			if shader_type == 'vertex':
			    cmd += ' -p vs_4_0'
			else:
			    cmd += ' -p ps_4_0'
			# cmd += ' -O 3'
		if sys.platform == 'win32':
			cmd = cmd.replace('/', '\\')
		print()
		print(cmd)
		ret = os.system(cmd)
		assert(ret == 0)
	print()
print("done.")
