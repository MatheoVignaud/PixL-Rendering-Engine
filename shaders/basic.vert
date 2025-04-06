#version 450

layout (location = 0) out vec2 TexCoord;

layout (set = 1 , binding = 0) uniform UBO
{
    vec2 pos;
    vec2 size;
} ubo;

const vec2 Vertex[6] = vec2[6](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0)
);

const vec2 TextureCoord[6] = vec2[6](
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0)
);

void main()
{
    vec2 pos = ubo.pos + ubo.size * Vertex[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
    TexCoord = TextureCoord[gl_VertexIndex];
}