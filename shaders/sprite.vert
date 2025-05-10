#version 450
layout (location = 0) out vec2 TexCoord;

layout (set = 1, binding = 0) uniform SpriteInfo
{
    vec2 position;
    float scale;
    int index;
};

const vec2 vertex[6] = vec2[6]
(
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
);


void main()
{
}