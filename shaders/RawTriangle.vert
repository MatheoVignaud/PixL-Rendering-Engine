#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout(set = 1, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;


layout (location = 0) out vec2 TexCoord;


void main()
{
    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}