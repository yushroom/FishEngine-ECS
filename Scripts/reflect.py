import clang.cindex
import sys, os
import json

if sys.platform == 'darwin':
	libclang_path = R'/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib'
	#libclang_path = R'/Users/yushroom/Downloads/llvm-3.9.1.src/build/lib/libclang.dylib'
else:
	libclang_path = R'D:\Library\LLVM\bin\libclang.dll'

clang.cindex.Config.set_library_file(libclang_path)
index = clang.cindex.Index.create()


class UnknownAttributeError(ValueError):
    pass

def DumpAst(sourcr_file_path, output_ast_path = 'out.ast'):
	include_path = (
			"../Include",
			"../ThirdParty",
			"../ThirdParty/bgfx/include",
			"../ThirdParty/bimg/include"
			)
	if sys.platform == 'darwin':
		pass
	else:
		clang_exe = r'D:\library\LLVM\bin\clang.exe'
	headers = ' '.join([f'-I"{os.path.abspath(x)}"' for x in include_path])
	# print(headers)
	cmd = f'{clang_exe} {sourcr_file_path} {headers} -x c++ -emit-ast -D__FISHENGINE_REFLECTION__ -std=c++17 -o {output_ast_path}'
	print(cmd)
	result = os.system(cmd)
	assert(result == 0)


Serialization_type = (
    'float',
    'double',
    'int',
    'uint32_t',
    'bool',
    'std::string',
    )

all_attributes = (
    'DisallowMultipleComponent',
    'HideInInspector',
    'Serializable',
    'NonSerializable',
    'ExecuteInEditMode',
    'AddComponentMenu',
    'RequireComponent',
    )

# class MemberDef:
# 	name
# 	type

# class ClassDef:
# 	name
# 	base_class
# 	scope_prefix
# 	header_file,
# 	members

def ParseClass(node, target_scope_prefix):
	if not node.is_definition():
		return
	scope_prefix = node.type.spelling.rsplit('::', 1)[0]
	# print(scope_prefix)
	if not node.type.spelling.startswith(target_scope_prefix):
		return

	class_name = node.type.spelling
	# print('class', class_name)
	header_file = node.location.file.name

	classdef = {}
	classdef["name"] = node.spelling
	classdef["scope_prefix"] = scope_prefix
	classdef["header_file"] = header_file
	classdef["base_class"] = None
	members = []

	for child in node.get_children():
		# print('\t', child.kind, child.spelling, child.type.spelling)
		if child.kind == clang.cindex.CursorKind.CXX_BASE_SPECIFIER:
			# print('\t', child.kind, child.spelling, child.type.spelling)
			# for cc in child.get_children():
			# 	base_class = cc.spelling    # get the last element
			base_class = child.type.spelling
			# print('\t\tbase class:', base_class)
			classdef["base_class"] = base_class
			pass

		elif child.kind == clang.cindex.CursorKind.CXX_METHOD:
			pass

		elif child.kind == clang.cindex.CursorKind.FUNCTION_TEMPLATE:
			pass

		elif child.kind == clang.cindex.CursorKind.FIELD_DECL:
			name = child.spelling
			member_type = child.type.spelling
			NonSerializable = False
			for c in child.get_children():
				if c.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
					if c.spelling not in all_attributes:
						print(c.spelling)
						raise UnknownAttributeError()
					elif c.spelling == 'NonSerializable':
						NonSerializable = True
			# print('\t\t', member_type, name)
			# for c in child.get_children():
			# 	# if c.kind == clang.cindex.CursorKind
			# 	pass
			if not NonSerializable:
				member = {}
				member["name"] = name
				member["type"] = member_type
				members.append(member)

	classdef["members"] = members
	return classdef



skip_cursor_types = (
	clang.cindex.CursorKind.ENUM_DECL,
	#clang.cindex.CursorKind.STRUCT_DECL,
	clang.cindex.CursorKind.FUNCTION_TEMPLATE,
	clang.cindex.CursorKind.UNEXPOSED_DECL,
	clang.cindex.CursorKind.TEMPLATE_REF
	)

classes = []

def FindTypes(node):
	# print(node.spelling)

	if (node.location.file is not None) and ('FishEngine' not in node.location.file.name):
		return
	if node.kind in skip_cursor_types:
		return

	if node.kind in (clang.cindex.CursorKind.CLASS_DECL, clang.cindex.CursorKind.STRUCT_DECL):
		classdef = ParseClass(node, 'FishEngine')
		if classdef is not None:
			classes.append(classdef)
		return

	for c in node.get_children():
		FindTypes(c)

def ExtractClasses(path):
	ast_path = './fe.ast'
	DumpAst(path, ast_path)
	tu = index.read(ast_path)
	FindTypes(tu.cursor)

	with open('components.json', 'w') as f:
		components = [c for c in classes if c['base_class'] == "FishEngine::Component"]
		print(json.dumps(components, indent=4), file=f)

	with open('singleton_components.json', 'w') as f:
		components = [c for c in classes if c['base_class'] == "FishEngine::SingletonComponent"]
		print(json.dumps(components, indent=4), file=f)

	with open('systems.json', 'w') as f:
		systems = [c for c in classes if c['base_class'] == "FishEngine::System"]
		print(json.dumps(systems, indent=4), file=f)


if __name__ == "__main__":
	classes = ExtractClasses("../Include/FishEngine.hpp")
	classes = ExtractClasses("../Include/FishEditor.hpp")