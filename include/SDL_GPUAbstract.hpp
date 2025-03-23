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

struct VertexBuffer_Struct
{
    SDL_GPUTransferBuffer *transferBuffer;
    SDL_GPUBuffer *buffer;

    void Destroy(SDL_GPUDevice *device)
    {
        SDL_ReleaseGPUBuffer(device, buffer);
    }

    void Update(SDL_GPUDevice *device, void *data, size_t size)
    {
        void *ptr = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
        SDL_memcpy(ptr, data, size);
        SDL_UnmapGPUTransferBuffer(device, transferBuffer);
        this->size = size;
    }
    void Upload(SDL_GPUDevice *device)
    {
        if (size == 0)
            return;
        SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
        SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

        SDL_GPUTransferBufferLocation transferBufferLocation = {};
        transferBufferLocation.transfer_buffer = transferBuffer;
        transferBufferLocation.offset = 0;

        SDL_GPUBufferRegion bufferRegion = {};
        bufferRegion.buffer = buffer;
        bufferRegion.offset = 0;
        bufferRegion.size = size;

        SDL_UploadToGPUBuffer(
            copyPass,
            &transferBufferLocation,
            &bufferRegion,
            false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    }
    void Bind(SDL_GPURenderPass *renderPass, uint32_t binding)
    {
        SDL_GPUBufferBinding vertexBufferBinding = {};
        vertexBufferBinding.buffer = buffer;
        vertexBufferBinding.offset = 0;
        SDL_BindGPUVertexBuffers(
            renderPass,
            binding,
            &vertexBufferBinding,
            1);
    }

private:
    size_t size = 0;
    friend VertexBuffer_Struct CreateVBO(SDL_GPUDevice *device, size_t size);
};

struct IndexBuffer_Struct
{
    SDL_GPUTransferBuffer *transferBuffer;
    SDL_GPUBuffer *buffer;

    void Destroy(SDL_GPUDevice *device)
    {
        SDL_ReleaseGPUBuffer(device, buffer);
    }
    void Update(SDL_GPUDevice *device, void *data, size_t size)
    {
        void *ptr = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
        SDL_memcpy(ptr, data, size);
        SDL_UnmapGPUTransferBuffer(device, transferBuffer);
        this->size = size;
    }
    void Upload(SDL_GPUDevice *device)
    {
        if (size == 0)
            return;
        SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
        SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

        SDL_GPUTransferBufferLocation transferBufferLocation = {};
        transferBufferLocation.transfer_buffer = transferBuffer;
        transferBufferLocation.offset = 0;

        SDL_GPUBufferRegion bufferRegion = {};
        bufferRegion.buffer = buffer;
        bufferRegion.offset = 0;
        bufferRegion.size = size;

        SDL_UploadToGPUBuffer(
            copyPass,
            &transferBufferLocation,
            &bufferRegion,
            false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    }
    void Bind(SDL_GPURenderPass *renderPass, SDL_GPUIndexElementSize indexElementSize)
    {
        SDL_GPUBufferBinding vertexBufferBinding = {};
        vertexBufferBinding.buffer = buffer;
        vertexBufferBinding.offset = 0;
        SDL_BindGPUIndexBuffer(
            renderPass,
            &vertexBufferBinding,
            indexElementSize);
    }

private:
    size_t size = 0;
    friend IndexBuffer_Struct CreateEBO(SDL_GPUDevice *device, size_t size);
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

struct DepthBuffer_Struct
{
    SDL_GPUTexture *texture;
    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfoClear;
    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfoLoad;
    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfoLoadReadOnly;
    SDL_GPUTextureSamplerBinding sampler;

    void Destroy(SDL_GPUDevice *device)
    {
        SDL_ReleaseGPUTexture(device, texture);
    }
};

SDL_GPUTextureSamplerBinding CreateSamplerFromImage(SDL_GPUDevice *device, std::string image_Path);
TransferBuffer_Struct CreateUBO(SDL_GPUDevice *device, size_t size);
VertexBuffer_Struct CreateVBO(SDL_GPUDevice *device, size_t size);
IndexBuffer_Struct CreateEBO(SDL_GPUDevice *device, size_t size);
SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    Shader_Struct *shader);
SDL_GPUGraphicsPipeline *CreatePipeline(SDL_GPUDevice *device, SDL_Window *window, Shader_Struct *vertexShader, Shader_Struct *fragmentShader, bool depthTest = false, SDL_GPUCompareOp compareOp = SDL_GPU_COMPAREOP_LESS, bool enable_depth_test = true, bool enable_depth_write = true);
DepthBuffer_Struct *CreateDepthBuffer(SDL_GPUDevice *device, uint32_t width, uint32_t height);