#version 450
layout (location = 0) out vec2 TexCoord;


const vec2 quad[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0)
);

const vec2 texturecoord[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

void main()
{
    gl_Position = vec4(quad[gl_VertexIndex], 0.0, 1.0);
    TexCoord = texturecoord[gl_VertexIndex];
}