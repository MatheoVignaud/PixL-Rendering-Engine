#pragma once

#include <SDL_GPUAbstract.hpp>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <vector>

#define DEFAULT_NAME "default"

struct Named_Pipeline
{
    std::string name;
    SDL_GPUGraphicsPipeline *pipeline;
    int vertexShader_UniformBuffer_Count = 0;
    int vertexShader_StorageBuffer_Count = 0;
    int fragmentShader_Sampler_Count = 0;
    int fragmentShader_UniformBuffer_Count = 0;
    int fragmentShader_StorageBuffer_Count = 0;
    int fragmentShader_StorageTexture_Count = 0;
    SDL_GPUCompareOp compareOp = SDL_GPU_COMPAREOP_ALWAYS;
    bool enable_depth_test = false;
    bool enable_depth_write = false;
    bool needDepthBuffer = false;
};

struct Named_DepthBuffer
{
    std::string name;
    DepthBuffer_Struct depthBuffer;
};

struct Named_Texture
{
    std::string name;
    SDL_GPUTextureSamplerBinding sampler;
};

struct Named_VBO
{
    std::string name;
    VertexBuffer_Struct vertexBuffer;
};

// Flags for PixL_Renderer_Init

// Rendering modes
#define PIXL_RENDERER_QUEUE_MODE 0x00000001 // Queue mode is the mode where the renderer will render the textures in the order they were added
#define PIXL_RENDERER_PLAN_MODE 0x00000002  // Plan mode is the mode where the renderer will render the textures in the order of their plan value

// Plan modes clear mode
#define PIXL_RENDERER_PLAN_MODE_AUTO_CLEAR 0x00000004   // Automatically clear the plan mode after rendering
#define PIXL_RENDERER_PLAN_MODE_MANUAL_CLEAR 0x00000008 // Manually clear the plan mode after rendering

// debug flags
#define PIXL_RENDERER_DEBUG_RED_WIRE 0x00000010

// PixL_Renderer main class
class PixL_Renderer
{
protected:
    PixL_Renderer();
    ~PixL_Renderer();
    static PixL_Renderer *_instance;

    int window_width = 0;
    int window_height = 0;

    // flags
    uint32_t _flags;
    SDL_Window *_window = nullptr;
    SDL_GPUDevice *_device = nullptr;

    SDL_GPUCommandBuffer *commandBuffer = nullptr;
    SDL_GPUTexture *swapchainTexture;
    SDL_GPURenderPass *renderPass = nullptr;

    bool drawing = false;

    uint32_t _drawCalls = 0;
    bool depthBufferClear = false;

    std::vector<Named_Pipeline> _pipelines;
    std::vector<Named_DepthBuffer> _depthBuffers;
    std::vector<Named_Texture> _textures;
    std::vector<Named_VBO> _VBOs;

    // friend functions
    friend int
    PixL_Renderer_Init(uint32_t flags);
    friend int PixL_Renderer_Quit();
    friend SDL_Window *CreateWindow(const char *title, int w, int h, Uint32 flags);
    friend SDL_Window *GetWindow();

    friend bool PixL_Draw(
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
        TransferBuffer_Struct *fragmentBufferSSBO);
    friend bool PixL_DrawIndexed(
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
        TransferBuffer_Struct *fragmentBufferSSBO);
    friend bool PixL_StartDraw();
    friend void PixL_SwapBuffers();
    friend bool PixL_CreatePipeline(std::string name, Shader_Struct *vertexShader, Shader_Struct *fragmentShader, bool depthTest, SDL_GPUCompareOp compareOp, bool enable_depth_test, bool enable_depth_write);
    friend bool PixL_DestroyPipeline(std::string name);
    friend bool PixL_CreateDepthBuffer(std::string name);
    friend bool PixL_DestroyDepthBuffer(std::string name);
    friend bool PixL_CreateTexture(std::string name, std::string imagePath);
    friend bool PixL_CreateBlankTexture(std::string name, int width, int height, SDL_GPUTextureUsageFlags usage);
    friend bool PixL_DestroyTexture(std::string name);

    friend bool PixL_StartRenderPass(std::string RenderTextureName, std::string DepthBufferName, bool needDepthBuffer, bool clearBuffers);
    friend bool PixL_EndRenderPass();
    friend bool PixL_Pipelines_Layout_Compatibility(std::string Pipeline1, std::string Pipeline2);

    friend bool PixL_CreateVBO(std::string name, size_t size);
    friend bool PixL_DestroyVBO(std::string name);
    friend VertexBuffer_Struct *PixL_GetVBO(std::string name);

    friend int PixL_GetDrawCalls();

    friend bool PixL_2D_Init(uint32_t flags);
    friend class PixL_2D;

    friend glm::vec2 PixL_GetWindowSize();

    // callbacks
    friend void PixL_Callback_WindowResized();
};

// function prototypes

int PixL_Renderer_Init(uint32_t flags);

int PixL_Renderer_Quit();

// Window functions
SDL_Window *CreateWindow(const char *title, int w, int h, Uint32 flags);
SDL_Window *GetWindow();

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
    TransferBuffer_Struct *fragmentBufferSSBO);

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
    TransferBuffer_Struct *fragmentBufferSSBO);

bool PixL_StartDraw();
void PixL_SwapBuffers();
bool PixL_CreatePipeline(std::string name, Shader_Struct *vertexShader, Shader_Struct *fragmentShader, bool depthTest = false, SDL_GPUCompareOp compareOp = SDL_GPU_COMPAREOP_LESS, bool enable_depth_test = true, bool enable_depth_write = true);
bool PixL_Pipelines_Layout_Compatibility(std::string Pipeline1, std::string Pipeline2);
bool PixL_DestroyPipeline(std::string name);
bool PixL_CreateDepthBuffer(std::string name);
bool PixL_DestroyDepthBuffer(std::string name);
bool PixL_CreateTexture(std::string name, std::string imagePath);
bool PixL_CreateBlankTexture(std::string name, int width, int height, SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_SAMPLER);
bool PixL_DestroyTexture(std::string name);
bool PixL_StartRenderPass(std::string RenderTextureName, std::string DepthBufferName, bool needDepthBuffer, bool clearBuffers = false);
bool PixL_EndRenderPass();
bool PixL_CreateVBO(std::string name, size_t size);
bool PixL_DestroyVBO(std::string name);
VertexBuffer_Struct *PixL_GetVBO(std::string name);

glm::vec2 PixL_GetWindowSize();

int PixL_GetDrawCalls();

// callbacks
void PixL_Callback_WindowResized();