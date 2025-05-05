// Fichier g�n�r� automatiquement
#include "const/generated_shaders.hpp"

// D�finitions des structures de shaders
Shader_Struct basic_fragment = {
    "basic.frag",
    SDL_GPU_SHADERSTAGE_FRAGMENT,
    0,
    0,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct basic_vertex = {
    "basic.vert",
    SDL_GPU_SHADERSTAGE_VERTEX,
    0,
    0,
    0,
    0,
    {
        {0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0},
    },
    {
        {SDL_GPUVertexBufferDescription{.slot = 0, .pitch = 8, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0}},
    },
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct batch_quads_fragment = {
    "batch_quads.frag",
    SDL_GPU_SHADERSTAGE_FRAGMENT,
    1,
    0,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct batch_quads_vertex = {
    "batch_quads.vert",
    SDL_GPU_SHADERSTAGE_VERTEX,
    0,
    1,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct fullscreen_texture_fragment = {
    "fullscreen_texture.frag",
    SDL_GPU_SHADERSTAGE_FRAGMENT,
    1,
    0,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct fullscreen_texture_vertex = {
    "fullscreen_texture.vert",
    SDL_GPU_SHADERSTAGE_VERTEX,
    0,
    0,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct sprite_fragment = {
    "sprite.frag",
    SDL_GPU_SHADERSTAGE_FRAGMENT,
    1,
    1,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};

Shader_Struct sprite_vertex = {
    "sprite.vert",
    SDL_GPU_SHADERSTAGE_VERTEX,
    0,
    1,
    0,
    0,
    {},
    {},
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};
