#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

struct UBO_Struct
{
    SDL_GPUTransferBuffer *transferBuffer;
    SDL_GPUBuffer *buffer;

    void Destroy(SDL_GPUDevice *device)
    {
        SDL_ReleaseGPUBuffer(device, buffer);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    }
};

SDL_GPUTextureSamplerBinding CreateSamplerFromImage(SDL_GPUDevice *device, std::string image_Path);
UBO_Struct CreateUBO(SDL_GPUDevice *device, size_t size);