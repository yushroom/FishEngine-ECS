from glob import glob
import os

shader_src_dir = "./Shaders"
out_dir = './Shaders/runtime'
shaderc = "../github/bgfx/.build/osx64_clang/bin/shadercDebug"

include_paths = ["../github/bgfx/src", "./Shaders/include"]

headers = ' '.join(['-i ' + x for x in include_paths])

platform = 'osx'
profile = 150

for shader_file in glob(os.path.join(shader_src_dir, "*.shader")):
    print(f'compile {shader_file}...')
    fn = os.path.split(shader_file)[-1]
    bn = fn.rsplit('.', 1)[0]
    for ext, shader_type, define in [('_vs.bin', 'vertex', 'VERTEX'), ('_fs.bin', 'fragment', 'FRAGMENT')]:
        compiled_shader_file = os.path.join(out_dir, bn + ext)
        cmd = f"{shaderc} {headers} --verbose -f {shader_file} -o {compiled_shader_file} --platform {platform} --type {shader_type} --define {define} --profile {profile}"
        # print(cmd)
        os.system(cmd)

# /Users/yushroom/program/github/bgfx/.build/osx64_clang/bin/shadercDebug -i /Users/yushroom/program/github/bgfx/src -i /Users/yushroom/program/FishEngine/Engine/Shaders/include 
# --verbose -f ./shader/PBR.sc -o ./shader/PBR_vs.bin --platform osx --type v --define VERTEX --profile 150
# /Users/yushroom/program/github/bgfx/.build/osx64_clang/bin/shadercDebug -i /Users/yushroom/program/github/bgfx/src -i /Users/yushroom/program/FishEngine/Engine/Shaders/include --verbose -f ./shader/PBR.sc -o ./shader/PBR_fs.bin --platform osx --type f --define FRAGMENT --profile 150

# /Users/yushroom/program/github/bgfx/.build/osx64_clang/bin/shadercDebug -i /Users/yushroom/program/github/bgfx/src -i /Users/yushroom/program/FishEngine/Engine/Shaders/include --verbose -f ./shader/Skybox.sc -o ./shader/Skybox_vs.bin --platform osx --type v --define VERTEX --profile 150
# /Users/yushroom/program/github/bgfx/.build/osx64_clang/bin/shadercDebug -i /Users/yushroom/program/github/bgfx/src -i /Users/yushroom/program/FishEngine/Engine/Shaders/include --verbose -f ./shader/Skybox.sc -o ./shader/Skybox_fs.bin --platform osx --type f --define FRAGMENT --profile 150
