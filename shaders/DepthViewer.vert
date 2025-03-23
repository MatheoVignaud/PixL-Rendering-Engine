#version 450


layout(location = 0) out vec2 fragUV;

const vec2 positions[4] = vec2[]( 
    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0)
);
const vec2 uvs[4] = vec2[]( 
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0)
);

const int indices[6] = int[](
    0, 1, 2,
    2, 1, 3
);

void main() {
    int index = indices[gl_VertexIndex];
    gl_Position = vec4(positions[index], 0.0, 1.0);
    fragUV = uvs[index];
}