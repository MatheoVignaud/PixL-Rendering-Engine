#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;


layout(set = 2, binding = 0) uniform sampler2D depthTexture;

const vec3 colorMod = vec3(0.5, 0.8, 0.6 );

void main() {
    float depth =1- texture(depthTexture, vec2(fragUV.x, 1.0 - fragUV.y)).r;
    float gamma = 2.2; 
    depth = pow(depth, 1.0 / gamma); 
    outColor = vec4(colorMod * depth, 1.0);
}

