#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;



layout(set = 1, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 inverseModel;
} mvp;


layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 FragPos;


void main()
{
    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    FragPos = vec3(mvp.model * vec4(aPos, 1.0));
    Normal = mat3(transpose(mvp.inverseModel)) * aNormal;
}