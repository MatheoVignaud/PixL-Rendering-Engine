#version 450
layout (location = 0)out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texture1;
layout (location = 0) in vec2 TexCoord;

void main()
{
    outColor = texture(texture1, TexCoord);
}