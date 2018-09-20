from glob import glob
import os

def Print(dir):
	include_root = '../Include/'
	with open(include_root + dir+'.hpp', 'w') as f:
		print("#pragma once", file=f)
		print('', file=f)
		for x in glob(include_root + dir+"/**/*.hpp", recursive=True):
			x = os.path.relpath(x, include_root)
			x = x.replace('\\', '/')
			print(f"#include \"{x}\"", file=f)

Print("FishEngine")
Print("FishEditor")