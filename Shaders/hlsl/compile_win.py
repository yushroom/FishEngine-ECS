from glob import glob
import os, sys

dxc = r'"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\dxc.exe"'
shader_include_dir = r'.\include'
output_dir = r'.\runtime'

if not os.path.exists(output_dir):
	os.makedirs(output_dir)

def RunCommand(cmd):
	print(cmd)
	ret = os.system(cmd)
	assert(ret == 0)

def Compile(hlsl_path):
	fn = os.path.splitext(os.path.basename(hlsl_path))[0]
	for type in ('vs', 'ps'):
		entry_point = type.upper()
		out_name = os.path.join(output_dir, f'{fn}_{type}.cso')
		# -H: Show header includes and nesting depth
		# -WX: Treat warnings as errors
		cmd = f"{dxc} -nologo -H -T {type}_6_0 -E {entry_point} {hlsl_path} -I {shader_include_dir} -Fo {out_name}"
		RunCommand(cmd)

for f in glob("*.hlsl"):
	print(f)
	Compile(f)