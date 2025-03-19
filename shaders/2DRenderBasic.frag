#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColorMod;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D myTexture;

void main() {
    outColor = texture(myTexture, vec2(fragTexCoord.x, 1.0 - fragTexCoord.y));
    outColor *= fragColorMod;
}
