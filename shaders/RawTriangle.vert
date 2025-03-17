#version 450

layout(location = 0) out vec2 fragTexCoord; 
layout(location = 1) out vec4 fragColorMod;

layout(set = 1, binding = 0) uniform UBO {
    vec2 center;
    float w;
    float h;
    vec2 srcCenter; // the center of the source image
    float srcW; // the width of the source image
    float srcH; // the height of the source image
    float rotation;
    vec2 rotation_center;
    bool flipX;
    bool flipY;
    vec4 colorMod;
} ubo;

const vec2 positions[4] = vec2[](
    vec2(-1, -1), 
    vec2( 1, -1), 
    vec2(-1,  1),
    vec2( 1,  1)
);


const vec2 texCoords[4] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

const uint indices[6] = uint[](
    0, 1, 2,
    2, 1, 3
);

const float PI = 3.14159265359;

vec2 rotate(vec2 vertex, float angle, vec2 rot_center) {
    float s = sin(angle);
    float c = cos(angle);

    vertex -= rot_center;

    vertex = vec2(vertex.x * c - vertex.y * s, vertex.x * s + vertex.y * c) + rot_center;
    return vertex;
}

vec2 GetTexCoord(vec2 src_center, vec2 src_size, uint index) {
    vec2 texCoord = texCoords[index];
    texCoord = texCoord * src_size   + (src_center - 0.5 );
    return texCoord;
}








void main() {
    uint index = indices[gl_VertexIndex];
    gl_Position = vec4(positions[index], 0.0, 1.0);
    
    fragTexCoord = GetTexCoord(ubo.srcCenter, vec2(ubo.srcW, ubo.srcH), index);
    
    
    gl_Position = vec4(rotate(gl_Position.xy, ubo.rotation, ubo.rotation_center), 0.0, 1.0);
    gl_Position.xy = gl_Position.xy * vec2(ubo.w, ubo.h);
    gl_Position.xy += ubo.center;

   
    if(ubo.flipX) {
        gl_Position.x = ubo.center.x - (gl_Position.x - ubo.center.x);
    }
    if(ubo.flipY) {
        gl_Position.y = ubo.center.y - (gl_Position.y - ubo.center.y);
    }

    fragColorMod = ubo.colorMod;
    

}
