#include <PixL_Renderer.hpp>

PixL_Renderer::PixL_Renderer()
{
}

PixL_Renderer *PixL_Renderer::_instance = nullptr;

PixL_Renderer::~PixL_Renderer()
{
    if (_window)
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
    if (_device)
    {
        SDL_DestroyGPUDevice(_device);
        _device = nullptr;
    }
    if (commandBuffer)
    {
    }
    for (auto &pipeline : _pipelines)
    {
        SDL_ReleaseGPUGraphicsPipeline(_device, pipeline.pipeline);
    }
    for (auto &texture : _textures)
    {
        SDL_ReleaseGPUTexture(_device, texture.sampler.texture);
        SDL_ReleaseGPUSampler(_device, texture.sampler.sampler);
    }
    for (auto &depthBuffer : _depthBuffers)
    {
        depthBuffer.depthBuffer.Destroy(_device);
    }
    SDL_DestroyGPUDevice(_device);
    SDL_DestroyWindow(_window);
}

int PixL_Renderer_Init(uint32_t flags)
{
    if (!PixL_Renderer::_instance)
    {
        PixL_Renderer::_instance = new PixL_Renderer();
    }
    PixL_Renderer::_instance->_flags = flags;

    return 0;
}

int PixL_Renderer_Quit()
{
    if (PixL_Renderer::_instance)
    {
        PixL_Renderer::_instance->~PixL_Renderer();
    }
    return 0;
}

SDL_Window *CreateWindow(const char *title, int w, int h, Uint32 flags)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return nullptr;
    }
    PixL_Renderer::_instance->_window = SDL_CreateWindow(title, w, h, flags);
    if (!PixL_Renderer::_instance->_window)
    {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return nullptr;
    }

    PixL_Renderer::_instance->_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!PixL_Renderer::_instance->_device)
    {
        SDL_Log("Could not create GPU device: %s", SDL_GetError());
        return nullptr;
    }

    if (!SDL_ClaimWindowForGPUDevice(PixL_Renderer::_instance->_device, PixL_Renderer::_instance->_window))
    {
        SDL_Log("Could not claim window for GPU device: %s", SDL_GetError());
        return nullptr;
    }

    return PixL_Renderer::_instance->_window;
}

SDL_Window *GetWindow()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return nullptr;
    }
    return PixL_Renderer::_instance->_window;
}

bool PixL_Draw(
    std::string PipelineName,
    std::string RenderTextureName, // "" for default swapchain texture , texture need to support render target
    std::string DepthBufferName,   // "" for default depth buffer
    int instanceCount,

    // vertex
    int vertexCount,
    VertexBuffer_Struct *vertexBuffer,
    std::pair<void *, size_t> vertexBufferUBOData,
    TransferBuffer_Struct *vertexBufferSSBO,

    // fragment
    std::vector<std::string> fragmentBufferSamplerNames, // bind samplers to the fragment shader , order is important
    std::pair<void *, size_t> fragmentBufferUBOData,
    TransferBuffer_Struct *fragmentBufferSSBO)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    if (!PixL_Renderer::_instance->drawing)
    {
        SDL_Log("PixL_Renderer not drawing. Call PixL_StartDraw() first.");
        return false;
    }

    // Find the pipeline
    Named_Pipeline *pipeline = nullptr;
    for (auto &p : PixL_Renderer::_instance->_pipelines)
    {
        if (p.name == PipelineName)
        {
            pipeline = &p;
            break;
        }
    }
    if (!pipeline)
    {
        SDL_Log("Pipeline %s not found", PipelineName.c_str());
        return false;
    }

    // Find the fragment buffer samplers
    std::vector<SDL_GPUTextureSamplerBinding> fragmentBufferSamplers;
    for (const auto &samplerName : fragmentBufferSamplerNames)
    {
        SDL_GPUTextureSamplerBinding sampler = {nullptr, nullptr};
        for (const auto &texture : PixL_Renderer::_instance->_textures)
        {
            if (texture.name == samplerName)
            {
                sampler = texture.sampler;
                break;
            }
        }
        if (!sampler.texture)
        {
            SDL_Log("Fragment buffer sampler %s not found", samplerName.c_str());
            throw std::runtime_error("Fragment buffer sampler not found");
            return false;
        }
        fragmentBufferSamplers.push_back(sampler);
    }
    if (fragmentBufferSamplers.size() != pipeline->fragmentShader_Sampler_Count)
    {
        SDL_Log("Fragment buffer sampler count mismatch. Expected %d, got %zu", pipeline->fragmentShader_Sampler_Count, fragmentBufferSamplers.size());
        return false;
    }

    // Bind the pipeline
    SDL_BindGPUGraphicsPipeline(PixL_Renderer::_instance->renderPass, pipeline->pipeline);
    // Bind the vertex buffer
    if (vertexBuffer)
    {
        vertexBuffer->Bind(PixL_Renderer::_instance->renderPass, 0);
    }

    // Bind the vertex uniform buffer
    if (vertexBuffer)
    {
        vertexBuffer->Bind(PixL_Renderer::_instance->renderPass, 0);
    }
    if (vertexBufferUBOData.first && pipeline->vertexShader_UniformBuffer_Count > 0)
    {
        SDL_PushGPUVertexUniformData(PixL_Renderer::_instance->commandBuffer, 0, vertexBufferUBOData.first, vertexBufferUBOData.second);
    }
    // Bind the vertex storage buffer

    // TODO : Bind the fragment samplers
    if (!fragmentBufferSamplers.empty())
    {
        for (size_t i = 0; i < fragmentBufferSamplers.size(); ++i)
        {
            SDL_BindGPUFragmentSamplers(PixL_Renderer::_instance->renderPass, i, &fragmentBufferSamplers[i], 1);
        }
    }
    // Bind the fragment uniform buffer
    if (fragmentBufferUBOData.first && pipeline->fragmentShader_UniformBuffer_Count > 0)
    {
        SDL_PushGPUFragmentUniformData(PixL_Renderer::_instance->commandBuffer, 0, fragmentBufferUBOData.first, fragmentBufferUBOData.second);
    }
    // TODO : Bind the fragment storage buffer

    SDL_DrawGPUPrimitives(
        PixL_Renderer::_instance->renderPass,
        vertexCount,
        instanceCount,
        0,
        0);

    if (!PixL_Renderer::_instance->depthBufferClear && pipeline->needDepthBuffer)
    {
        PixL_Renderer::_instance->depthBufferClear = true;
    }
    PixL_Renderer::_instance->_drawCalls++;
    // std::cout << "Draw with pipeline " << PipelineName << " with " << vertexCount << " vertices and " << instanceCount << " instances" << std::endl;
    return true;
}

bool PixL_DrawIndexed(
    std::string PipelineName,
    std::string RenderTextureName, // "" for default swapchain texture , texture need to support render target
    std::string DepthBufferName,   // "" for default depth buffer
    int instanceCount,

    // Indices
    int indexCount,
    IndexBuffer_Struct *indexBuffer,

    // vertex
    int vertexCount,
    VertexBuffer_Struct *vertexBuffer,
    std::pair<void *, size_t> vertexBufferUBOData,
    TransferBuffer_Struct *vertexBufferSSBO,

    // fragment
    std::vector<std::string> fragmentBufferSamplerNames, // bind samplers to the fragment shader , order is important
    std::pair<void *, size_t> fragmentBufferUBOData,
    TransferBuffer_Struct *fragmentBufferSSBO)
{

    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    if (!PixL_Renderer::_instance->drawing)
    {
        SDL_Log("PixL_Renderer not drawing. Call PixL_StartDraw() first.");
        return false;
    }

    // Find the pipeline
    Named_Pipeline *pipeline = nullptr;
    for (auto &p : PixL_Renderer::_instance->_pipelines)
    {
        if (p.name == PipelineName)
        {
            pipeline = &p;
            break;
        }
    }
    if (!pipeline)
    {
        SDL_Log("Pipeline %s not found", PipelineName.c_str());
        return false;
    }

    // Find the fragment buffer samplers
    std::vector<SDL_GPUTextureSamplerBinding> fragmentBufferSamplers;
    for (const auto &samplerName : fragmentBufferSamplerNames)
    {
        SDL_GPUTextureSamplerBinding sampler = {nullptr, nullptr};
        for (const auto &texture : PixL_Renderer::_instance->_textures)
        {
            if (texture.name == samplerName)
            {
                sampler = texture.sampler;
                break;
            }
        }
        if (!sampler.texture)
        {
            SDL_Log("Fragment buffer sampler %s not found", samplerName.c_str());
            return false;
        }
        fragmentBufferSamplers.push_back(sampler);
    }
    if (fragmentBufferSamplers.size() != pipeline->fragmentShader_Sampler_Count)
    {
        SDL_Log("Fragment buffer sampler count mismatch. Expected %d, got %zu", pipeline->fragmentShader_Sampler_Count, fragmentBufferSamplers.size());
        return false;
    }

    // Bind the pipeline
    SDL_BindGPUGraphicsPipeline(PixL_Renderer::_instance->renderPass, pipeline->pipeline);
    // Bind the vertex buffer
    if (vertexBuffer)
    {
        vertexBuffer->Bind(PixL_Renderer::_instance->renderPass, 0);
    }

    // Bind the vertex uniform buffer
    if (vertexBuffer)
    {
        vertexBuffer->Bind(PixL_Renderer::_instance->renderPass, 0);
    }
    if (vertexBufferUBOData.first && pipeline->vertexShader_UniformBuffer_Count > 0)
    {
        SDL_PushGPUVertexUniformData(PixL_Renderer::_instance->commandBuffer, 0, vertexBufferUBOData.first, vertexBufferUBOData.second);
    }
    // Bind the vertex storage buffer

    // TODO : Bind the fragment samplers
    if (!fragmentBufferSamplers.empty())
    {
        for (size_t i = 0; i < fragmentBufferSamplers.size(); ++i)
        {
            SDL_BindGPUFragmentSamplers(PixL_Renderer::_instance->renderPass, i, &fragmentBufferSamplers[i], 1);
        }
    }
    // Bind the fragment uniform buffer
    if (fragmentBufferUBOData.first && pipeline->fragmentShader_UniformBuffer_Count > 0)
    {
        SDL_PushGPUFragmentUniformData(PixL_Renderer::_instance->commandBuffer, 0, fragmentBufferUBOData.first, fragmentBufferUBOData.second);
    }
    // TODO : Bind the fragment storage buffer

    if (indexBuffer)
    {
        indexBuffer->Bind(PixL_Renderer::_instance->renderPass, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    SDL_DrawGPUIndexedPrimitives(
        PixL_Renderer::_instance->renderPass,
        indexCount,
        instanceCount,
        0,
        0,
        0);
    PixL_Renderer::_instance->_drawCalls++;

    if (!PixL_Renderer::_instance->depthBufferClear & pipeline->needDepthBuffer)
    {
        PixL_Renderer::_instance->depthBufferClear = true;
    }

    return true;
}

bool PixL_StartDraw()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    if (PixL_Renderer::_instance->drawing)
    {
        SDL_Log("PixL_Renderer already drawing. Call PixL_SwapBuffers() first.");
        return false;
    }
    PixL_Renderer::_instance->drawing = true;
    PixL_Renderer::_instance->commandBuffer = SDL_AcquireGPUCommandBuffer(PixL_Renderer::_instance->_device);
    if (!PixL_Renderer::_instance->commandBuffer)
    {
        SDL_Log("Could not acquire command buffer: %s", SDL_GetError());
        return false;
    }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(PixL_Renderer::_instance->commandBuffer, PixL_Renderer::_instance->_window, &PixL_Renderer::_instance->swapchainTexture, NULL, NULL))
    {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return false;
    }
    if (!PixL_Renderer::_instance->swapchainTexture)
    {
        SDL_Log("Swapchain texture is null");
        return false;
    }

    PixL_Renderer::_instance->_drawCalls = 0;

    return true;
}

void PixL_SwapBuffers()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return;
    }
    SDL_SubmitGPUCommandBuffer(PixL_Renderer::_instance->commandBuffer);

    PixL_Renderer::_instance->drawing = false;
}

bool PixL_CreatePipeline(std::string name, Shader_Struct *vertexShader, Shader_Struct *fragmentShader, bool depthTest, SDL_GPUCompareOp compareOp, bool enable_depth_test, bool enable_depth_write)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    // Check if the pipeline already exists
    for (const auto &pipeline : PixL_Renderer::_instance->_pipelines)
    {
        if (pipeline.name == name)
        {
            SDL_Log("Pipeline %s already exists", name);
            return false;
        }
    }

    SDL_GPUGraphicsPipeline *pipeline = CreatePipeline(PixL_Renderer::_instance->_device, PixL_Renderer::_instance->_window, vertexShader, fragmentShader, depthTest, compareOp, enable_depth_test, enable_depth_write);
    if (!pipeline)
    {
        SDL_Log("Could not create pipeline: %s", SDL_GetError());
        return false;
    }

    Named_Pipeline named_pipeline = {name, pipeline};
    named_pipeline.fragmentShader_Sampler_Count = fragmentShader->sampler_Count;
    named_pipeline.fragmentShader_UniformBuffer_Count = fragmentShader->uniform_Buffer_Count;
    named_pipeline.fragmentShader_StorageBuffer_Count = fragmentShader->storage_Buffer_Count;
    named_pipeline.fragmentShader_StorageTexture_Count = fragmentShader->storage_Texture_Count;
    named_pipeline.vertexShader_UniformBuffer_Count = vertexShader->uniform_Buffer_Count;
    named_pipeline.vertexShader_StorageBuffer_Count = vertexShader->storage_Buffer_Count;
    named_pipeline.compareOp = compareOp;
    named_pipeline.enable_depth_test = enable_depth_test;
    named_pipeline.enable_depth_write = enable_depth_write;
    named_pipeline.needDepthBuffer = depthTest;
    PixL_Renderer::_instance->_pipelines.push_back(named_pipeline);

    SDL_Log("Pipeline %s created successfully", name.c_str());

    return true;
}

bool PixL_DestroyPipeline(std::string name)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    for (auto it = PixL_Renderer::_instance->_pipelines.begin(); it != PixL_Renderer::_instance->_pipelines.end(); ++it)
    {
        if (it->name == name)
        {
            SDL_ReleaseGPUGraphicsPipeline(PixL_Renderer::_instance->_device, it->pipeline);
            PixL_Renderer::_instance->_pipelines.erase(it);
            SDL_Log("Pipeline %s destroyed successfully", name);
            return true;
        }
    }
    SDL_Log("Pipeline %s not found", name);
    return false;
}

bool PixL_CreateDepthBuffer(std::string name)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    // Check if the depth buffer already exists
    for (const auto &depthBuffer : PixL_Renderer::_instance->_depthBuffers)
    {
        if (depthBuffer.name == name)
        {
            SDL_Log("Depth buffer %s already exists", name);
            return false;
        }
    }
    return false;
}

bool PixL_DestroyDepthBuffer(std::string name)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    // if the depth buffer is default, do not destroy it
    if (name == "default")
    {
        SDL_Log("Cannot destroy default depth buffer");
        return false;
    }
    for (auto it = PixL_Renderer::_instance->_depthBuffers.begin(); it != PixL_Renderer::_instance->_depthBuffers.end(); ++it)
    {
        if (it->name == name)
        {
            it->depthBuffer.Destroy(PixL_Renderer::_instance->_device);
            PixL_Renderer::_instance->_depthBuffers.erase(it);
            SDL_Log("Depth buffer %s destroyed successfully", name);
            return true;
        }
    }
    SDL_Log("Depth buffer %s not found", name);
    return false;
}

bool PixL_CreateTexture(std::string name, std::string imagePath)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    // Check if the texture already exists
    for (const auto &texture : PixL_Renderer::_instance->_textures)
    {
        if (texture.name == name)
        {
            SDL_Log("Texture %s already exists", name);
            return false;
        }
    }

    SDL_GPUTextureSamplerBinding sampler = CreateSamplerFromImage(PixL_Renderer::_instance->_device, imagePath);
    if (!sampler.texture)
    {
        SDL_Log("Could not create texture: %s", SDL_GetError());
        return false;
    }

    Named_Texture named_texture = {name, sampler};
    PixL_Renderer::_instance->_textures.push_back(named_texture);

    SDL_Log("Texture %s created successfully", name.c_str());

    return true;
}

bool PixL_CreateBlankTexture(std::string name, int width, int height, SDL_GPUTextureUsageFlags usage)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    // Check if the texture already exists
    for (const auto &texture : PixL_Renderer::_instance->_textures)
    {
        if (texture.name == name)
        {
            SDL_Log("Texture %s already exists", name);
            return false;
        }
    }

    SDL_GPUTextureSamplerBinding sampler = CreateBlankSampler(PixL_Renderer::_instance->_device, SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM, width, height, usage);
    if (!sampler.texture)
    {
        SDL_Log("Could not create texture: %s", SDL_GetError());
        return false;
    }

    Named_Texture named_texture = {name, sampler};
    PixL_Renderer::_instance->_textures.push_back(named_texture);

    SDL_Log("Texture %s created successfully", name.c_str());

    return true;
}

bool PixL_DestroyTexture(std::string name)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    for (auto it = PixL_Renderer::_instance->_textures.begin(); it != PixL_Renderer::_instance->_textures.end(); ++it)
    {
        if (it->name == name)
        {
            SDL_ReleaseGPUTexture(PixL_Renderer::_instance->_device, it->sampler.texture);
            SDL_ReleaseGPUSampler(PixL_Renderer::_instance->_device, it->sampler.sampler);
            PixL_Renderer::_instance->_textures.erase(it);
            SDL_Log("Texture %s destroyed successfully", name.c_str());
            return true;
        }
    }
    SDL_Log("Texture %s not found", name.c_str());
    return false;
}

bool PixL_StartRenderPass(std::string RenderTextureName, std::string DepthBufferName, bool needDepthBuffer, bool clearBuffers)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    if (!PixL_Renderer::_instance->drawing)
    {
        SDL_Log("PixL_Renderer not drawing. Call PixL_StartDraw() first.");
        return false;
    }

    // Find the render texture
    SDL_GPUTexture *renderTexture = nullptr;
    if (!RenderTextureName.empty() || RenderTextureName == DEFAULT_NAME)
    {
        for (auto &texture : PixL_Renderer::_instance->_textures)
        {
            if (texture.name == RenderTextureName)
            {
                renderTexture = texture.sampler.texture;
                break;
            }
        }
        if (!renderTexture)
        {
            SDL_Log("Render texture %s not found", RenderTextureName.c_str());
            return false;
        }
    }
    else
    {
        // Use the swapchain texture
        renderTexture = PixL_Renderer::_instance->swapchainTexture;
    }

    if (!renderTexture)
    {
        SDL_Log("Render texture is null");
        return false;
    }

    Named_DepthBuffer *depthBuffer = nullptr;
    if (needDepthBuffer)
    {
        bool defaultDepthBuffer = false;
        if (DepthBufferName.empty())
        {
            defaultDepthBuffer = true;
        }

        for (auto &db : PixL_Renderer::_instance->_depthBuffers)
        {
            if (db.name == (defaultDepthBuffer ? "default" : DepthBufferName))
            {
                depthBuffer = &db;
                break;
            }
        }

        if (!depthBuffer)
        {
            SDL_Log("Depth buffer %s not found", DepthBufferName.c_str());
            return false;
        }
    }

    // Create the render pass but take in account the number of draw calls
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = renderTexture;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 0.0f};
    if (PixL_Renderer::_instance->_drawCalls == 0 || clearBuffers)
    {
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    }
    else
    {
        colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    }
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {0};
    if (depthBuffer)
    {
        if (PixL_Renderer::_instance->depthBufferClear)
        {
            depthStencilTargetInfo = depthBuffer->depthBuffer.depthStencilTargetInfoLoad;
        }
        else
        {
            depthStencilTargetInfo = depthBuffer->depthBuffer.depthStencilTargetInfoClear;
        }
    }

    PixL_Renderer::_instance->renderPass = SDL_BeginGPURenderPass(PixL_Renderer::_instance->commandBuffer,
                                                                  &colorTargetInfo,
                                                                  1,
                                                                  depthBuffer ? &depthStencilTargetInfo : NULL);

    // std::cout << "Starting render pass" << std::endl;
    return true;
}

bool PixL_EndRenderPass()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    if (!PixL_Renderer::_instance->drawing)
    {
        SDL_Log("PixL_Renderer not drawing. Call PixL_StartDraw() first.");
        return false;
    }
    if (PixL_Renderer::_instance->renderPass == nullptr)
    {
        SDL_Log("Render pass not started. Call PixL_StartRenderPass() first.");
        throw std::runtime_error("Render pass not started. Call PixL_StartRenderPass() first.");
        return false;
    }

    // std::cout << "Ending render pass" << std::endl;
    SDL_EndGPURenderPass(PixL_Renderer::_instance->renderPass);
    PixL_Renderer::_instance->renderPass = nullptr;
    return true;
}

bool PixL_CreateVBO(std::string name, size_t size)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    // Check if the VBO already exists
    for (const auto &vbo : PixL_Renderer::_instance->_VBOs)
    {
        if (vbo.name == name)
        {
            SDL_Log("VBO %s already exists", name);
            return false;
        }
    }
    // Create the VBO
    VertexBuffer_Struct vbo = CreateVBO(PixL_Renderer::_instance->_device, size);
    if (!vbo.buffer)
    {
        SDL_Log("Could not create VBO: %s", SDL_GetError());
        return false;
    }
    Named_VBO named_vbo = {name, vbo};
    PixL_Renderer::_instance->_VBOs.push_back(named_vbo);
    SDL_Log("VBO %s created successfully", name.c_str());
    return true;
}

bool PixL_DestroyVBO(std::string name)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    for (auto it = PixL_Renderer::_instance->_VBOs.begin(); it != PixL_Renderer::_instance->_VBOs.end(); ++it)
    {
        if (it->name == name)
        {
            it->vertexBuffer.Destroy(PixL_Renderer::_instance->_device);
            PixL_Renderer::_instance->_VBOs.erase(it);
            SDL_Log("VBO %s destroyed successfully", name.c_str());
            return true;
        }
    }
    SDL_Log("VBO %s not found", name.c_str());
    return false;
}

VertexBuffer_Struct *PixL_GetVBO(std::string name)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return nullptr;
    }
    for (auto &vbo : PixL_Renderer::_instance->_VBOs)
    {
        if (vbo.name == name)
        {
            return &vbo.vertexBuffer;
        }
    }
    SDL_Log("VBO %s not found", name.c_str());
    throw std::runtime_error("VBO not found");
    return nullptr;
}

bool PixL_Pipelines_Layout_Compatibility(std::string Pipeline1, std::string Pipeline2)
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return false;
    }
    Named_Pipeline *pipeline1 = nullptr;
    Named_Pipeline *pipeline2 = nullptr;
    for (auto &p : PixL_Renderer::_instance->_pipelines)
    {
        if (p.name == Pipeline1)
        {
            pipeline1 = &p;
        }
        if (p.name == Pipeline2)
        {
            pipeline2 = &p;
        }
    }
    if (!pipeline1 || !pipeline2)
    {
        SDL_Log("Pipeline %s or %s not found", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }

    if (pipeline1->vertexShader_UniformBuffer_Count != pipeline2->vertexShader_UniformBuffer_Count)
    {
        SDL_Log("Pipeline %s and %s have different vertex shader uniform buffer counts", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->vertexShader_StorageBuffer_Count != pipeline2->vertexShader_StorageBuffer_Count)
    {
        SDL_Log("Pipeline %s and %s have different vertex shader storage buffer counts", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->fragmentShader_Sampler_Count != pipeline2->fragmentShader_Sampler_Count)
    {
        SDL_Log("Pipeline %s and %s have different fragment shader sampler counts", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->fragmentShader_UniformBuffer_Count != pipeline2->fragmentShader_UniformBuffer_Count)
    {
        SDL_Log("Pipeline %s and %s have different fragment shader uniform buffer counts", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->fragmentShader_StorageBuffer_Count != pipeline2->fragmentShader_StorageBuffer_Count)
    {
        SDL_Log("Pipeline %s and %s have different fragment shader storage buffer counts", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->fragmentShader_StorageTexture_Count != pipeline2->fragmentShader_StorageTexture_Count)
    {
        SDL_Log("Pipeline %s and %s have different fragment shader storage texture counts", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->compareOp != pipeline2->compareOp)
    {
        SDL_Log("Pipeline %s and %s have different compare ops", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->enable_depth_test != pipeline2->enable_depth_test)
    {
        SDL_Log("Pipeline %s and %s have different depth test settings", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->enable_depth_write != pipeline2->enable_depth_write)
    {
        SDL_Log("Pipeline %s and %s have different depth write settings", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }
    if (pipeline1->needDepthBuffer != pipeline2->needDepthBuffer)
    {
        SDL_Log("Pipeline %s and %s have different depth buffer settings", Pipeline1.c_str(), Pipeline2.c_str());
        return false;
    }

    return true;
}

glm::vec2 PixL_GetWindowSize()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return glm::vec2(0, 0);
    }
    return glm::vec2(PixL_Renderer::_instance->window_width, PixL_Renderer::_instance->window_height);
}

int PixL_GetDrawCalls()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return -1;
    }
    return PixL_Renderer::_instance->_drawCalls;
}

void PixL_Callback_WindowResized()
{
    if (!PixL_Renderer::_instance)
    {
        SDL_Log("PixL_Renderer not initialized. Call PixL_Renderer_Init() first.");
        return;
    }
    SDL_GetWindowSize(PixL_Renderer::_instance->_window, &PixL_Renderer::_instance->window_width, &PixL_Renderer::_instance->window_height);
}
