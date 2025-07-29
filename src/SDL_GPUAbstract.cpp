#include <SDL_GPUAbstract.hpp>

SDL_GPUTextureSamplerBinding CreateSamplerFromImage(SDL_GPUDevice *device, std::string image_Path, uint32_t *getWidth, uint32_t *getHeight)
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

    if (getWidth != nullptr)
    {
        *getWidth = surface->w;
    }
    if (getHeight != nullptr)
    {
        *getHeight = surface->h;
    }

    SDL_UnlockSurface(surface);

    // create a texture from the surface
    SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (uint32_t)surface->w * (uint32_t)surface->h * 4};

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
        true);

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
}
SDL_GPUTextureSamplerBinding CreateBlankSampler(SDL_GPUDevice *device, SDL_GPUTextureFormat format, uint32_t width, uint32_t height, SDL_GPUTextureUsageFlags usage)
{
    SDL_GPUTextureCreateInfo textureCreateInfo = {};
    textureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
    textureCreateInfo.format = format;
    textureCreateInfo.width = width;
    textureCreateInfo.height = height;
    textureCreateInfo.layer_count_or_depth = 1;
    textureCreateInfo.num_levels = 1;
    textureCreateInfo.usage = usage;

    SDL_GPUTexture *texture = SDL_CreateGPUTexture(
        device,
        &textureCreateInfo);

    if (texture == NULL)
    {
        SDL_Log("Failed to create texture!");
        throw std::runtime_error("Failed to create texture!");
        return {};
    }

    SDL_GPUSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.min_filter = SDL_GPU_FILTER_NEAREST;
    samplerCreateInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
    samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    SDL_GPUSampler *sampler = SDL_CreateGPUSampler(
        device,
        &samplerCreateInfo);

    return {texture, sampler};
};

TransferBuffer_Struct CreateUBO(SDL_GPUDevice *device, size_t size)
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
}

TransferBuffer_Struct CreateSSBO(SDL_GPUDevice *device, size_t size)
{
    SDL_GPUTransferBufferCreateInfo ssboTransferBufferCreateInfo = {};
    ssboTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    ssboTransferBufferCreateInfo.size = size;

    SDL_GPUTransferBuffer *ssboTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &ssboTransferBufferCreateInfo);

    SDL_GPUBufferCreateInfo ssboBufferCreateInfo = {};
    ssboBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    ssboBufferCreateInfo.size = size;

    SDL_GPUBuffer *SSBOBuffer = SDL_CreateGPUBuffer(
        device,
        &ssboBufferCreateInfo);

    TransferBuffer_Struct ssboTransferBuffer_Struct = {};
    ssboTransferBuffer_Struct.transferBuffer = ssboTransferBuffer;
    ssboTransferBuffer_Struct.buffer = SSBOBuffer;

    return ssboTransferBuffer_Struct;
}

VertexBuffer_Struct CreateVBO(SDL_GPUDevice *device, size_t size)
{
    SDL_GPUTransferBufferCreateInfo vboTransferBufferCreateInfo = {};
    vboTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    vboTransferBufferCreateInfo.size = size;

    SDL_GPUTransferBuffer *vboTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &vboTransferBufferCreateInfo);

    SDL_GPUBufferCreateInfo vboBufferCreateInfo = {};
    vboBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vboBufferCreateInfo.size = size;

    SDL_GPUBuffer *VBOBuffer = SDL_CreateGPUBuffer(
        device,
        &vboBufferCreateInfo);
    SDL_UnmapGPUTransferBuffer(device, vboTransferBuffer);

    VertexBuffer_Struct VBOBuffer_Struct = {};
    VBOBuffer_Struct.transferBuffer = vboTransferBuffer;
    VBOBuffer_Struct.buffer = VBOBuffer;
    VBOBuffer_Struct.size = size;

    return VBOBuffer_Struct;
}

bool UpdateSSBO(SDL_GPUDevice *device, TransferBuffer_Struct *ssbo, void *data, size_t size)
{

    // check if the SSBO is valid
    if (ssbo == nullptr || ssbo->transferBuffer == nullptr || ssbo->buffer == nullptr)
    {
        SDL_Log("Invalid SSBO provided!");
        return false;
    }

    // update the SSBO data
    void *ptr = SDL_MapGPUTransferBuffer(device, ssbo->transferBuffer, true);
    if (ptr == NULL)
    {
        SDL_Log("Failed to map SSBO transfer buffer!");
        return false;
    }
    SDL_memcpy(ptr, data, size);
    SDL_UnmapGPUTransferBuffer(device, ssbo->transferBuffer);

    // upload the SSBO data to the GPU
    SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_GPUTransferBufferLocation transferBufferLocation = {};
    transferBufferLocation.transfer_buffer = ssbo->transferBuffer;
    transferBufferLocation.offset = 0;
    SDL_GPUBufferRegion bufferRegion = {};
    bufferRegion.buffer = ssbo->buffer;
    bufferRegion.offset = 0;
    bufferRegion.size = size;
    SDL_UploadToGPUBuffer(
        copyPass,
        &transferBufferLocation,
        &bufferRegion,
        false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    return true;
}

IndexBuffer_Struct CreateEBO(SDL_GPUDevice *device, size_t size)
{
    SDL_GPUTransferBufferCreateInfo eboTransferBufferCreateInfo = {};
    eboTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    eboTransferBufferCreateInfo.size = size;

    SDL_GPUTransferBuffer *eboTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &eboTransferBufferCreateInfo);

    SDL_GPUBufferCreateInfo eboBufferCreateInfo = {};
    eboBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    eboBufferCreateInfo.size = size;

    SDL_GPUBuffer *EBOBuffer = SDL_CreateGPUBuffer(
        device,
        &eboBufferCreateInfo);
    SDL_UnmapGPUTransferBuffer(device, eboTransferBuffer);

    IndexBuffer_Struct EBOBuffer_Struct = {};
    EBOBuffer_Struct.transferBuffer = eboTransferBuffer;
    EBOBuffer_Struct.buffer = EBOBuffer;
    EBOBuffer_Struct.size = size;

    return EBOBuffer_Struct;
}

SDL_GPUShader *LoadShader(SDL_GPUDevice *device, Shader_Struct *shader)
{
    char fullPath[256];
    const char *entrypoint;

    SDL_snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.spv", BasePath, shader->shader_Path.c_str());
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
    entrypoint = "main";

    size_t codeSize;
    void *code = SDL_LoadFile(fullPath, &codeSize);
    if (code == NULL)
    {
        SDL_Log("Failed to load shader from disk! %s", fullPath);
        return NULL;
    }

    SDL_GPUShaderCreateInfo shaderInfo{};
    shaderInfo.code = static_cast<const uint8_t *>(code);
    shaderInfo.code_size = codeSize;
    shaderInfo.entrypoint = entrypoint;
    shaderInfo.format = format;
    shaderInfo.stage = shader->shader_Stage;
    shaderInfo.num_samplers = shader->sampler_Count;
    shaderInfo.num_uniform_buffers = shader->uniform_Buffer_Count;
    shaderInfo.num_storage_buffers = shader->storage_Buffer_Count;
    shaderInfo.num_storage_textures = shader->storage_Texture_Count;

    SDL_GPUShader *GPU_shader = SDL_CreateGPUShader(device, &shaderInfo);

    if (GPU_shader == NULL)
    {
        SDL_Log("Failed to create shader!");
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);
    return GPU_shader;
}

SDL_GPUGraphicsPipeline *CreatePipeline(SDL_GPUDevice *device, SDL_Window *window, Shader_Struct *vertexShader, Shader_Struct *fragmentShader, bool depthTest, SDL_GPUCompareOp compareOp, bool enable_depth_test, bool enable_depth_write)
{
    if (vertexShader == NULL || fragmentShader == NULL)
    {
        SDL_Log("Shaders are NULL!");
        return NULL;
    }

    if (vertexShader->shader_Stage != SDL_GPU_SHADERSTAGE_VERTEX || fragmentShader->shader_Stage != SDL_GPU_SHADERSTAGE_FRAGMENT)
    {
        SDL_Log("Shaders are not of the correct type!");
        return NULL;
    }

    SDL_GPUShader *vertexShaderGPU = LoadShader(device, vertexShader);
    SDL_GPUShader *fragmentShaderGPU = LoadShader(device, fragmentShader);

    SDL_GPUColorTargetDescription target_desc = {};
    target_desc.format = SDL_GetGPUSwapchainTextureFormat(device, window);
    target_desc.blend_state = {};
    target_desc.blend_state.enable_blend = true;
    target_desc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    target_desc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    target_desc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    target_desc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    target_desc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    target_desc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

    SDL_GPUGraphicsPipelineTargetInfo pipeline_target_info = {};
    pipeline_target_info.num_color_targets = 1;
    pipeline_target_info.color_target_descriptions = &target_desc;
    pipeline_target_info.has_depth_stencil_target = depthTest;

    SDL_GPUDepthStencilState depth_stencil_state = {};

    if (depthTest)
    {
        pipeline_target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
        depth_stencil_state.enable_depth_test = enable_depth_test;
        depth_stencil_state.enable_depth_write = enable_depth_write;
        depth_stencil_state.enable_stencil_test = false;
        depth_stencil_state.compare_op = compareOp;
        depth_stencil_state.write_mask = 0xFF;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.target_info = pipeline_target_info;
    pipelineCreateInfo.vertex_shader = vertexShaderGPU;
    pipelineCreateInfo.fragment_shader = fragmentShaderGPU;
    pipelineCreateInfo.primitive_type = vertexShader->primitive_Type;
    if (vertexShader->vertexAttributes.size() > 0)
    {
        pipelineCreateInfo.vertex_input_state = {};
        pipelineCreateInfo.vertex_input_state.num_vertex_attributes = vertexShader->vertexAttributes.size();
        pipelineCreateInfo.vertex_input_state.vertex_attributes = vertexShader->vertexAttributes.data();
        pipelineCreateInfo.vertex_input_state.num_vertex_buffers = vertexShader->vertexBuffers.size();
        pipelineCreateInfo.vertex_input_state.vertex_buffer_descriptions = vertexShader->vertexBuffers.data();
    }
    pipelineCreateInfo.rasterizer_state = {};
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineCreateInfo.depth_stencil_state = depth_stencil_state;

    SDL_GPUGraphicsPipeline *Pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);
    if (Pipeline == NULL)
    {
        std::cerr << "Pipeline could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    SDL_ReleaseGPUShader(device, vertexShaderGPU);
    SDL_ReleaseGPUShader(device, fragmentShaderGPU);

    return Pipeline;
}
DepthBuffer_Struct *CreateDepthBuffer(SDL_GPUDevice *device, uint32_t width, uint32_t height)
{
    SDL_GPUTextureCreateInfo depthBufferCreateInfo = {};
    depthBufferCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
    depthBufferCreateInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    depthBufferCreateInfo.width = width;
    depthBufferCreateInfo.height = height;
    depthBufferCreateInfo.layer_count_or_depth = 1;
    depthBufferCreateInfo.num_levels = 1;
    depthBufferCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

    DepthBuffer_Struct *DepthBuffer = new DepthBuffer_Struct();
    DepthBuffer->texture = SDL_CreateGPUTexture(
        device,
        &depthBufferCreateInfo);

    DepthBuffer->depthStencilTargetInfoClear = {};
    DepthBuffer->depthStencilTargetInfoClear.texture = DepthBuffer->texture;
    DepthBuffer->depthStencilTargetInfoClear.clear_depth = 1.0f;
    DepthBuffer->depthStencilTargetInfoClear.load_op = SDL_GPU_LOADOP_CLEAR;
    DepthBuffer->depthStencilTargetInfoClear.store_op = SDL_GPU_STOREOP_STORE;
    DepthBuffer->depthStencilTargetInfoClear.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    DepthBuffer->depthStencilTargetInfoClear.stencil_store_op = SDL_GPU_STOREOP_STORE;
    DepthBuffer->depthStencilTargetInfoClear.cycle = true;
    DepthBuffer->depthStencilTargetInfoClear.clear_stencil = 0;

    DepthBuffer->depthStencilTargetInfoLoad = DepthBuffer->depthStencilTargetInfoClear;
    DepthBuffer->depthStencilTargetInfoLoad.load_op = SDL_GPU_LOADOP_LOAD;
    DepthBuffer->depthStencilTargetInfoLoad.stencil_load_op = SDL_GPU_LOADOP_LOAD;
    DepthBuffer->depthStencilTargetInfoLoad.cycle = false;

    DepthBuffer->depthStencilTargetInfoLoadReadOnly = DepthBuffer->depthStencilTargetInfoLoad;
    DepthBuffer->depthStencilTargetInfoLoadReadOnly.store_op = SDL_GPU_STOREOP_DONT_CARE;
    DepthBuffer->depthStencilTargetInfoLoadReadOnly.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

    DepthBuffer->sampler = {};
    DepthBuffer->sampler.texture = DepthBuffer->texture;

    SDL_GPUSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.min_filter = SDL_GPU_FILTER_NEAREST;
    samplerCreateInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
    samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;

    DepthBuffer->sampler.sampler = SDL_CreateGPUSampler(
        device,
        &samplerCreateInfo);

    return DepthBuffer;
}

bool UpdateTexture(SDL_GPUDevice *device, SDL_GPUTextureSamplerBinding *texture, uint32_t width, uint32_t height, void *data, size_t size)
{
    SDL_GPUTransferBufferCreateInfo transferBufferInfo = {};
    transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferBufferInfo.size = size;

    SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &transferBufferInfo);

    if (transferBuffer == NULL)
    {
        SDL_Log("Failed to create transfer buffer!");
        return false;
    }

    void *transferBufferPtr = SDL_MapGPUTransferBuffer(
        device,
        transferBuffer,
        true);
    SDL_memcpy(transferBufferPtr, data, size);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUTextureTransferInfo transferInfo = {};
    transferInfo.transfer_buffer = transferBuffer;
    transferInfo.offset = 0;

    SDL_GPUTextureRegion region = {};
    region.texture = texture->texture;
    region.w = width;
    region.h = height;
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
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    return true;
}
