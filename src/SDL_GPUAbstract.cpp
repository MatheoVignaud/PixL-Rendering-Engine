#include <SDL_GPUAbstract.hpp>

SDL_GPUTextureSamplerBinding CreateSamplerFromImage(SDL_GPUDevice *device, std::string image_Path)
{
    SDL_Surface *surface = IMG_Load(image_Path.c_str());
    if (surface == NULL)
    {
        SDL_Log("Could not load image data!");
        return {};
    }

    // get surface pixel format
    SDL_PixelFormat format = surface->format;

    // check if the surface is not in the desired format
    if (format != SDL_PIXELFORMAT_ABGR8888)
    {
        // convert the surface to the desired format
        SDL_Surface *convertedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);
        if (convertedSurface == NULL)
        {
            SDL_Log("Could not convert surface to desired format!");
            SDL_DestroySurface(surface);
            return {};
        }

        // free the original surface
        SDL_DestroySurface(surface);

        // set the converted surface as the new surface
        surface = convertedSurface;
    }

    // create a texture from the surface
    SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = surface->w * surface->h * 4};

    SDL_GPUTransferBuffer *textureTransferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

    if (textureTransferBuffer == NULL)
    {
        SDL_Log("Could not create transfer buffer!");
        SDL_DestroySurface(surface);
        return {};
    }

    void *textureTransferPtr = SDL_MapGPUTransferBuffer(
        device,
        textureTransferBuffer,
        false);

    SDL_memcpy(textureTransferPtr, surface->pixels, surface->w * surface->h * 4);
    SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

    SDL_GPUTextureCreateInfo textureCreateInfo = {};
    textureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
    textureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureCreateInfo.width = surface->w;
    textureCreateInfo.height = surface->h;
    textureCreateInfo.layer_count_or_depth = 1;
    textureCreateInfo.num_levels = 1;
    textureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    SDL_GPUTexture *Texture = SDL_CreateGPUTexture(
        device,
        &textureCreateInfo);

    SDL_GPUSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.min_filter = SDL_GPU_FILTER_NEAREST;
    samplerCreateInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
    samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    SDL_GPUSampler *Sampler = SDL_CreateGPUSampler(
        device,
        &samplerCreateInfo);

    SDL_GPUTextureTransferInfo transferInfo = {};
    transferInfo.transfer_buffer = textureTransferBuffer;
    transferInfo.offset = 0;

    SDL_GPUTextureRegion region = {};
    region.texture = Texture;
    region.w = surface->w;
    region.h = surface->h;
    region.d = 1;

    SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_UploadToGPUTexture(
        copyPass,
        &transferInfo,
        &region,
        false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    SDL_DestroySurface(surface);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);

    return {Texture, Sampler};
};

UBO_Struct CreateUBO(SDL_GPUDevice *device, size_t size)
{
    SDL_GPUTransferBufferCreateInfo uboTransferBufferCreateInfo = {};
    uboTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    uboTransferBufferCreateInfo.size = size;

    SDL_GPUTransferBuffer *uboTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &uboTransferBufferCreateInfo);

    SDL_GPUBufferCreateInfo uboBufferCreateInfo = {};
    uboBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    uboBufferCreateInfo.size = size;

    SDL_GPUBuffer *UBOBuffer = SDL_CreateGPUBuffer(
        device,
        &uboBufferCreateInfo);

    return {uboTransferBuffer, UBOBuffer};
};
