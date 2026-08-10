from pathlib import Path

shaders_source_dir = Path("resources/shaders")
shaders_compilation_dir = Path("engine/graphics/include")
shaders_compilation_src_dir = Path("engine/graphics/src")

with open(shaders_compilation_dir / "compiled_shaders.hpp", "w+") as file:
    file.write("#pragma once\n\n#include <string>\n\n")
    file.write("struct CompiledShaders {\n")
    for file_path in shaders_source_dir.iterdir():
        if file_path.name.endswith(".wgsl"):
            shader_contents = file_path.read_text()
            file.write(
                f" static std::string {file_path.name.replace('.wgsl', '')};\n\n"
            )
    file.write("};\n")

with open(shaders_compilation_src_dir / "compiled_shaders.cpp", "w+") as file:
    file.write('#include "compiled_shaders.hpp"\n\n')
    for file_path in shaders_source_dir.iterdir():
        if file_path.name.endswith(".wgsl"):
            shader_contents = file_path.read_text()
            file.write(
                f'std::string CompiledShaders::{file_path.name.replace(".wgsl", "")} = R"({shader_contents})";\n\n'
            )
