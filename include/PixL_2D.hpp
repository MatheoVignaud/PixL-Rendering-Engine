#pragma once

#include <PixL_Renderer.hpp>
#include <glm/glm.hpp>

enum PixL_2D_RenderMode
{
    QUEUE_MODE,
    PLAN_MODE,
};

struct PixL_2D_Sprite_Sheet
{
    std::string textureName;
    glm::vec2 spriteSheetSize = {0, 0}; // size of the sprite sheet in sprites
};

struct PixL_2D_Sprite
{
    uint16_t zIndex = 0;
    int spriteIndex = 0;
    glm::vec2 position = {0, 0};
    float scaleX = 1;
};

struct PixL_2D_Instanced_Sprite
{
    PixL_2D_Sprite_Sheet *spriteSheet = nullptr;
    std::vector<PixL_2D_Sprite> sprites = {};
};

struct Sprite_UBO
{
    alignas(8) glm::vec2 position;
    alignas(4) float scale;
    alignas(4) int index;
};

struct Quad_UBO
{
    alignas(8) glm::vec2 position;
    alignas(8) glm::vec2 size;
};

struct SpriteSheet_UBO
{
    alignas(8) glm::vec2 size;
};

class PixL_2D
{
protected:
    PixL_2D();
    ~PixL_2D();
    static PixL_2D *_instance;

    PixL_2D_RenderMode _renderMode = PixL_2D_RenderMode::QUEUE_MODE;
    TransferBuffer_Struct _spriteBuffer;

    TransferBuffer_Struct _quadUBO;
    TransferBuffer_Struct _BatchquadUBO;

    bool DrawSprite(
        std::string PipelineName,
        PixL_2D_Sprite *sprite);
    bool DrawInstancedSprite(
        std::string PipelineName,
        PixL_2D_Instanced_Sprite *sprite);
    bool DrawTexturedQuad(
        std::string TextureName,
        glm::vec2 position,
        glm::vec2 size = {1, 1});
    bool DrawTexturedQuadBatch(
        std::string TextureName,
        std::vector<std::pair<Quad_UBO, int>> quads);

    // friend functions
    friend bool PixL_2D_Init(uint32_t flags);
    friend bool PixL_2D_DrawSprite(std::string PipelineName, PixL_2D_Sprite *sprite);
    friend bool PixL_2D_DrawInstancedSprite(std::string PipelineName, PixL_2D_Instanced_Sprite *sprite);
    friend bool PixL_2D_DrawTexturedQuad(std::string TextureName, glm::vec2 position, glm::vec2 size);
    friend bool PixL_2D_DrawTexturedQuadBatch(std::string TextureName, std::vector<std::pair<Quad_UBO, int>> quads);
};

bool PixL_2D_Init(uint32_t flags);
bool PixL_2D_DrawSprite(std::string PipelineName, PixL_2D_Sprite *sprite);
bool PixL_2D_DrawInstancedSprite(std::string PipelineName, PixL_2D_Instanced_Sprite *sprite);
bool PixL_2D_DrawTexturedQuad(std::string TextureName, glm::vec2 position, glm::vec2 size = {1, 1});
bool PixL_2D_DrawTexturedQuadBatch(std::string TextureName, std::vector<std::pair<Quad_UBO, int>> quads);