#include <PixL_2D.hpp>
#include <const/generated_shaders.hpp>

PixL_2D::PixL_2D()
{
    // load all shaders
    PixL_CreatePipeline("basic", &basic_vertex, &basic_fragment, false, SDL_GPU_COMPAREOP_LESS, false, false);
    PixL_CreatePipeline("batch_quads", &batch_quads_vertex, &batch_quads_fragment, false, SDL_GPU_COMPAREOP_LESS, false, false);

    this->_quadUBO = CreateUBO(PixL_Renderer::_instance->_device, sizeof(Quad_UBO));
    this->_BatchquadUBO = CreateUBO(PixL_Renderer::_instance->_device, sizeof(Quad_UBO) * 1000); // 1000 quads
}

PixL_2D::~PixL_2D()
{
    if (_instance)
    {
        delete _instance;
        _instance = nullptr;
    }
}

bool PixL_2D::DrawSprite(std::string PipelineName, PixL_2D_Sprite *sprite)
{
    return true;
}

bool PixL_2D::DrawInstancedSprite(std::string PipelineName, PixL_2D_Instanced_Sprite *sprite)
{
    // sort sprites by zIndex
    std::sort(sprite->sprites.begin(), sprite->sprites.end(), [](PixL_2D_Sprite a, PixL_2D_Sprite b)
              { return a.zIndex < b.zIndex; });

    // create ubo data
    std::vector<Sprite_UBO> uboData;
    uboData.reserve(sprite->sprites.size());
    return true;
}

bool PixL_2D::DrawTexturedQuad(std::string TextureName, glm::vec2 position, glm::vec2 size)
{
    // create UBO data
    struct Sprite_UBO
    {
        alignas(8) glm::vec2 position;
        alignas(8) glm::vec2 size;
    } uboData = {position, size};

    PixL_Draw(
        "basic",
        "",
        "",
        1,
        6,
        nullptr,
        {&uboData, sizeof(glm::vec2) * 2},
        nullptr,
        {TextureName},
        {nullptr, 0},
        nullptr);
    return true;
}

bool PixL_2D::DrawTexturedQuadBatch(std::string TextureName, std::vector<std::pair<Quad_UBO, int>> quads)
{
    std::cout << "Drawing " << 0 << " quads" << std::endl;
    // sort quads by zIndex
    std::sort(quads.begin(), quads.end(), [](std::pair<Quad_UBO, int> a, std::pair<Quad_UBO, int> b)
              { return a.second < b.second; });

    PixL_StartRenderPass("", "", false);

    while (quads.size() > 0)
    {
        int numQuads = quads.size() <= 1000 ? quads.size() : 1000;
        // create ubo data
        Quad_UBO uboData[1000];
        for (int i = 0; i < numQuads; i++)
        {
            uboData[i] = quads[i].first;
        }
        quads.erase(quads.begin(), quads.begin() + numQuads);

        // draw the batch
        PixL_Draw(
            "batch_quads",
            "",
            "",
            numQuads,
            6,
            nullptr,
            {&uboData, sizeof(Quad_UBO) * numQuads},
            nullptr,
            {TextureName},
            {nullptr, 0},
            nullptr);
    }
    PixL_EndRenderPass();
    return true;
}

PixL_2D *PixL_2D::_instance = nullptr;

bool PixL_2D_Init(uint32_t flags)
{
    if (PixL_Renderer::_instance == nullptr)
    {
        PixL_Renderer::_instance = new PixL_Renderer();
    }

    if (PixL_2D::_instance == nullptr)
    {
        PixL_2D::_instance = new PixL_2D();
    }

    return true;
}

bool PixL_2D_DrawTexturedQuad(std::string TextureName, glm::vec2 position, glm::vec2 size)
{
    return PixL_2D::_instance->DrawTexturedQuad(TextureName, position, size);
}

bool PixL_2D_DrawTexturedQuadBatch(std::string TextureName, std::vector<std::pair<Quad_UBO, int>> quads)
{
    return PixL_2D::_instance->DrawTexturedQuadBatch(TextureName, quads);
}
