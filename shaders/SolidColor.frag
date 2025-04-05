#version 450
layout (location = 0)out vec4 outColor;
  
layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 FragPos;

layout(set = 2, binding = 0) uniform sampler2D diffuse_map;
layout(set = 2, binding = 1) uniform sampler2D specular_map;

layout(set = 3, binding = 0) uniform Light{
    vec3 lightColor;
    vec3 lightPos;
    vec3 lightDir;
    vec3 viewPos;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
}light;


void main()
{
    float distance = length(light.lightPos - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 norm = normalize(Normal);
    // Diffuse shading
    vec3 lightDir = normalize(light.lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.lightColor * vec3(texture(diffuse_map, TexCoord)) * attenuation;

    // ambient shading
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(texture(diffuse_map, TexCoord)) * light.lightColor * attenuation;

    // specular shading
    vec3 viewDir = normalize(light.viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 2 * spec * light.lightColor * vec3(texture(specular_map, TexCoord)) * attenuation;

    float theta = dot(lightDir, normalize(light.lightDir));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;
    outColor = vec4(ambient + diffuse + specular, 1.0);
        
}