#version 450
layout (location = 0)out vec4 outColor;
  
layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 FragPos;

layout(set = 3, binding = 0) uniform Light{
    vec3 lightColor;
    vec3 lightPos;
    vec3 viewPos;
}light;



void main()
{
    outColor = vec4(light.lightColor, 1.0);
}