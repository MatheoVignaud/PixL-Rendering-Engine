import os
import re

def map_glsl_to_gpu_format(glsl_type):
    glsl_type = glsl_type.lower()
    mapping = {
        "float": "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT",
        "vec2": "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2",
        "vec3": "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3",
        "vec4": "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4",
        "int": "SDL_GPU_VERTEXELEMENTFORMAT_INT",
        "ivec2": "SDL_GPU_VERTEXELEMENTFORMAT_INT2",
        "ivec3": "SDL_GPU_VERTEXELEMENTFORMAT_INT3",
        "ivec4": "SDL_GPU_VERTEXELEMENTFORMAT_INT4",
        "uint": "SDL_GPU_VERTEXELEMENTFORMAT_UINT",
        "uvec2": "SDL_GPU_VERTEXELEMENTFORMAT_UINT2",
        "uvec3": "SDL_GPU_VERTEXELEMENTFORMAT_UINT3",
        "uvec4": "SDL_GPU_VERTEXELEMENTFORMAT_UINT4",
        "half2": "SDL_GPU_VERTEXELEMENTFORMAT_HALF2",
        "half4": "SDL_GPU_VERTEXELEMENTFORMAT_HALF4",
        "byte2": "SDL_GPU_VERTEXELEMENTFORMAT_BYTE2",
        "byte4": "SDL_GPU_VERTEXELEMENTFORMAT_BYTE4",
        "ubyte2": "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2",
        "ubyte4": "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4",
        "byte2_norm": "SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM",
        "byte4_norm": "SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM",
        "ubyte2_norm": "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM",
        "ubyte4_norm": "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM",
        "short2": "SDL_GPU_VERTEXELEMENTFORMAT_SHORT2",
        "short4": "SDL_GPU_VERTEXELEMENTFORMAT_SHORT4",
        "ushort2": "SDL_GPU_VERTEXELEMENTFORMAT_USHORT2",
        "ushort4": "SDL_GPU_VERTEXELEMENTFORMAT_USHORT4",
        "short2_norm": "SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM",
        "short4_norm": "SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM",
        "ushort2_norm": "SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM",
        "ushort4_norm": "SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM",
    }
    return mapping.get(glsl_type, "SDL_GPU_VERTEXELEMENTFORMAT_INVALID")

class ShaderStruct:
    def __init__(self, shader_path, shader_stage):
        self.shader_Path = shader_path
        self.shader_Stage = shader_stage
        self.sampler_Count = 0
        self.uniform_Buffer_Count = 0
        self.storage_Buffer_Count = 0
        self.storage_Texture_Count = 0
        self.vertexAttributes = []
        self.vertexBuffers = []
        self.primitive_Type = "SDL_GPU_PRIMITIVETYPE_TRIANGLELIST"

def list_shader_files(shader_dir):
    shader_files = []
    for root, _, files in os.walk(shader_dir):
        for file in files:
            if file.endswith('.vert') or file.endswith('.frag'):
                shader_files.append(os.path.join(root, file))
    return shader_files

def get_format_size(format_enum):
    size_map = {
        "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3": 12,
        "SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4": 16,
        "SDL_GPU_VERTEXELEMENTFORMAT_INT": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_INT2": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_INT3": 12,
        "SDL_GPU_VERTEXELEMENTFORMAT_INT4": 16,
        "SDL_GPU_VERTEXELEMENTFORMAT_UINT": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_UINT2": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_UINT3": 12,
        "SDL_GPU_VERTEXELEMENTFORMAT_UINT4": 16,
        "SDL_GPU_VERTEXELEMENTFORMAT_BYTE2": 2,
        "SDL_GPU_VERTEXELEMENTFORMAT_BYTE4": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2": 2,
        "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM": 2,
        "SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM": 2,
        "SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_SHORT2": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_SHORT4": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_USHORT2": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_USHORT4": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM": 8,
        "SDL_GPU_VERTEXELEMENTFORMAT_HALF2": 4,
        "SDL_GPU_VERTEXELEMENTFORMAT_HALF4": 8,
    }
    return size_map.get(format_enum, 0)

def parse_shader(shader_path):
    with open(shader_path, 'r') as f:
        lines = f.readlines()

    shader_stage = "SDL_GPU_SHADERSTAGE_VERTEX" if shader_path.endswith(".vert") else "SDL_GPU_SHADERSTAGE_FRAGMENT"
    shader_struct = ShaderStruct(shader_path, shader_stage)

    location_pattern = re.compile(r"layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+(\w+)\s+(\w+);")
    ubo_pattern_vertex = re.compile(r"layout\s*\([^)]*set\s*=\s*1[^)]*\)\s*uniform")
    ubo_pattern_fragment = re.compile(r"layout\s*\([^)]*set\s*=\s*3[^)]*\)\s*uniform")
    ssbo_pattern = re.compile(r"layout\s*\(\s*std430\s*,\s*binding\s*=\s*\d+\s*\)\s*buffer")
    sampler_pattern = re.compile(r"uniform\s+sampler2D")
    image_pattern = re.compile(r"layout\s*\(\s*binding\s*=\s*\d+\s*\)\s*uniform\s+image2D")

    current_offset = 0

    # Traiter chaque ligne pour extraire les attributs
    for line in lines:
        # Vertex attributes
        if shader_path.endswith(".vert"):
            match = location_pattern.search(line)
            if match:
                location = int(match.group(1))
                glsl_type = match.group(2)
                format_enum = map_glsl_to_gpu_format(glsl_type)
                attr_size = get_format_size(format_enum)
                attr = f'{{ {location}, 0, {format_enum}, {current_offset} }}'
                shader_struct.vertexAttributes.append(attr)
                current_offset += attr_size

        # UBO, SSBO, Sampler, Image2D
        if ubo_pattern_vertex.search(line) or ubo_pattern_fragment.search(line):
            shader_struct.uniform_Buffer_Count += 1
        if ssbo_pattern.search(line):
            shader_struct.storage_Buffer_Count += 1
        if sampler_pattern.search(line):
            shader_struct.sampler_Count += 1
        if image_pattern.search(line):
            shader_struct.storage_Texture_Count += 1

    # Regrouper les attributs en un seul buffer de vertex
    if shader_path.endswith(".vert") and current_offset > 0:
        vertex_buffer_desc = f"{{ SDL_GPUVertexBufferDescription{{.slot = 0, .pitch = {current_offset}, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0 }} }}"
        shader_struct.vertexBuffers.append(vertex_buffer_desc)

    return shader_struct

def sanitize_name(path):
    name = os.path.splitext(os.path.basename(path))[0]
    return name.replace(".", "_").replace("-", "_")

def generate_hpp(shader_structs, output_path="include/const/generated_shaders.hpp"):
    with open(output_path, 'w') as f:
        f.write("// Auto-generated shader instances\n")
        f.write("#pragma once\n\n")
        f.write("#include <SDL_GPUAbstract.hpp>\n\n")

        for s in shader_structs:
            name = sanitize_name(s.shader_Path)
            if s.shader_Stage == "SDL_GPU_SHADERSTAGE_VERTEX":
                name += "_vertex"
            elif s.shader_Stage == "SDL_GPU_SHADERSTAGE_FRAGMENT":
                name += "_fragment"
            path_str = s.shader_Path.replace("\\", "/").replace("shaders/", "")
            f.write(f"Shader_Struct {name} = {{\n")
            f.write(f'    "{path_str}",\n')
            f.write(f"    {s.shader_Stage},\n")
            f.write(f"    {s.sampler_Count},\n")
            f.write(f"    {s.uniform_Buffer_Count},\n")
            f.write(f"    {s.storage_Buffer_Count},\n")
            f.write(f"    {s.storage_Texture_Count},\n")

            f.write("    {\n")
            for attr in s.vertexAttributes:
                f.write(f"        {attr},\n")
            f.write("    },\n")

            f.write("    {\n")
            for vb in s.vertexBuffers:
                f.write(f"        {vb},\n")
            f.write("    },\n")

            f.write(f"    {s.primitive_Type}\n")
            f.write("};\n\n")

# 🔧 Exécution
shader_dir = "shaders"
shaders = list_shader_files(shader_dir)
shader_structs = [parse_shader(shader) for shader in shaders]
generate_hpp(shader_structs)
