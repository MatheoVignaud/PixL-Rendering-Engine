#pragma once

#include <SDL_GPUAbstract.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <camera.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex
{
    alignas(16) glm::vec3 Position;
    alignas(16) glm::vec3 Normal;
    alignas(8) glm::vec2 TexCoord;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
    SDL_GPUTextureSamplerBinding samplerBinding;
};

class Mesh
{
public:
    Mesh(SDL_GPUDevice *device, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    /*  Mesh Data  */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    void Bind(SDL_GPURenderPass *renderPass, uint32_t binding);

private:
    VertexBuffer_Struct VBO;
    IndexBuffer_Struct EBO;
    void setupMesh(SDL_GPUDevice *device);
};

class Model
{
public:
    Model(SDL_GPUDevice *device, char *path)
    {
        loadModel(device, path);
    }
    void Draw(SDL_GPUCommandBuffer *commandBuffer, DepthBuffer_Struct *depth_buffer, SDL_GPUTexture *swapchainTexture, SDL_GPUGraphicsPipeline *Pipeline, MVP mvp, LightUBO lightUBO)
    {

        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            std::cout << "Drawing mesh " << i << std::endl;
            SDL_GPURenderPass *renderPass = NULL;
            SDL_GPUColorTargetInfo colorTargetInfo = {0};

            if (i == 0)
            {

                colorTargetInfo.texture = swapchainTexture;
                colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
                colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
                colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
                SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(
                    commandBuffer,
                    &colorTargetInfo,
                    1,
                    &depth_buffer->depthStencilTargetInfoClear);
            }
            else
            {
                renderPass = SDL_BeginGPURenderPass(
                    commandBuffer,
                    &colorTargetInfo,
                    1,
                    &depth_buffer->depthStencilTargetInfoLoad);
            }

            SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
            meshes[i].Bind(renderPass, 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(MVP));
            SDL_PushGPUFragmentUniformData(commandBuffer, 0, &lightUBO, sizeof(LightUBO));
            SDL_DrawGPUIndexedPrimitives(renderPass, meshes[i].indices.size(), SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, 0, 0, 0);
            SDL_EndGPURenderPass(renderPass);

            if (i == 0)
            {
                colorTargetInfo = {0};
                colorTargetInfo.texture = swapchainTexture;
                colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
                colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
                colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            }
            // sleep(1);
            _sleep(30);
        }
    }

private:
    // model data
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded;
    std::string directory;

    void loadModel(SDL_GPUDevice *device, std::string path);
    void processNode(SDL_GPUDevice *device, aiNode *node, const aiScene *scene);
    Mesh processMesh(SDL_GPUDevice *device, aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(SDL_GPUDevice *device, aiMaterial *mat, aiTextureType type,
                                              std::string typeName);
};