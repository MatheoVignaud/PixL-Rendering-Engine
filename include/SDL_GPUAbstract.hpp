#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include <vector>

static const char *BasePath = SDL_GetBasePath();

struct TransferBuffer_Struct
{
    SDL_GPUTransferBuffer *transferBuffer;
    SDL_GPUBuffer *buffer;

    void Destroy(SDL_GPUDevice *device)
    {
        SDL_ReleaseGPUBuffer(device, buffer);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    }
};

struct Shader_Struct
{
    std::string shader_Path;
    SDL_GPUShaderStage shader_Stage;
    uint32_t sampler_Count;
    uint32_t uniform_Buffer_Count;
    uint32_t storage_Buffer_Count;
    uint32_t storage_Texture_Count;

    // vertex shader specific
    std::vector<SDL_GPUVertexAttribute> vertexAttributes = {};
    std::vector<SDL_GPUVertexBufferDescription> vertexBuffers = {};

    SDL_GPUPrimitiveType primitive_Type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
};

SDL_GPUTextureSamplerBinding CreateSamplerFromImage(SDL_GPUDevice *device, std::string image_Path);
TransferBuffer_Struct CreateUBO(SDL_GPUDevice *device, size_t size);
SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    Shader_Struct *shader);
SDL_GPUGraphicsPipeline *CreatePipeline(SDL_GPUDevice *device, SDL_Window *window, Shader_Struct *vertexShader, Shader_Struct *fragmentShader);