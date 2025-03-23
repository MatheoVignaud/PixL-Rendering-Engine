#version 450
layout (location = 0)out vec4 outColor;
  
layout (location = 0) in vec2 TexCoord;

layout(set = 2, binding = 0) uniform sampler2D texture1;
layout(set = 2, binding = 1) uniform sampler2D texture2;

void main()
{
    outColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}