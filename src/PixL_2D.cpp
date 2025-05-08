#include <PixL_2D.hpp>

PixL_2D::PixL_2D()
{
    if (PixL_Renderer::_instance == nullptr)
    {
        throw std::runtime_error("PixL_Renderer not initialized!");
    }

    // Initialize the default pipeline for 2D rendering
    use_default_PixL_2D_Pipeline();

    PixL_CreatePipeline("Fullscreen", &fullscreen_texture_vertex, &fullscreen_texture_fragment, false, SDL_GPU_COMPAREOP_LESS, false, false);
}

PixL_2D *PixL_2D::_instance = nullptr;

PixL_2D::~PixL_2D()
{
    // Clean up all layers and their textures
    for (auto &layer : layers)
    {
        PixL_DestroyTexture(layer.second._TextureName);
    }
    layers.clear();
    drawables_order.clear();
}

void PixL_2D::initLayer(uint8_t layer_id, uint16_t width, uint16_t height)
{
    if (layers.find(layer_id) != layers.end())
    {
        return; // Layer already initialized
    }

    Layer layer;

    int layer_width, layer_height;
    if (width == 0 || height == 0)
    {
        layer_width = PixL_Renderer::_instance->window_width;
        layer_height = PixL_Renderer::_instance->window_height;
    }
    else
    {
        layer_width = width;
        layer_height = height;
    }

    PixL_CreateBlankTexture("Layer_" + std::to_string(layer_id), layer_width, layer_height, SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER);

    layer._TextureName = "Layer_" + std::to_string(layer_id);

    layer.drawables.clear();
    layer.effects.clear();
    layers[layer_id] = layer;
}

void PixL_2D::renderLayer(uint8_t layer_id)
{
    // Check if the layer exists
    if (layers.find(layer_id) == layers.end())
    {
        return; // Layer not found
    }

    Layer &layer = layers[layer_id];

    std::vector<DrawableWrapper> drawables = layer.drawables;
    drawables.insert(drawables.end(), layer.pipeline_drawables.begin(), layer.pipeline_drawables.end());
    std::sort(drawables.begin(), drawables.end(), [](const DrawableWrapper &a, const DrawableWrapper &b)
              { return a.z_index < b.z_index; });

    bool needToClear = true;
    if (cleared_layers.find(layer_id) != cleared_layers.end())
    {
        needToClear = false;
    }
    else
    {
        cleared_layers.insert(layer_id);
    }

    bool render_pass_started = false;

    for (const auto &drawable : layer.drawables)
    {
        if (last_pipeline_name != drawable.pipeline)
        {
            if (last_pipeline_name != "")
            {
                PixL_EndRenderPass();
            }
            if (needToClear)
            {
                PixL_StartRenderPass(layer._TextureName, "", false, true);
            }
            else
            {
                PixL_StartRenderPass(layer._TextureName, "", false, false);
            }
            render_pass_started = true;
            last_pipeline_name = drawable.pipeline;
        }

        if (drawable.callback)
        {
            drawable.callback(drawable.user_data);
        }
    }
    if (render_pass_started)
    {
        PixL_EndRenderPass();
    }
    last_pipeline_name = "";
}

void PixL_2D::applyLayerEffects(uint8_t layer_id)
{
    // Check if the layer exists
    if (layers.find(layer_id) == layers.end())
    {
        return; // Layer not found
    }

    Layer &layer = layers[layer_id];

    std::vector<LayerEffect> effects = layer.effects;
    effects.insert(effects.end(), layer.pipeline_effects.begin(), layer.pipeline_effects.end());
    std::sort(effects.begin(), effects.end(), [](const LayerEffect &a, const LayerEffect &b)
              { return a.z_index < b.z_index; });

    bool needToClear = true;
    if (cleared_layers.find(layer_id) != cleared_layers.end())
    {
        needToClear = false;
    }
    else
    {
        cleared_layers.insert(layer_id);
    }

    bool render_pass_started = false;

    for (const auto &effect : effects)
    {
        if (last_pipeline_name != effect.pipeline)
        {
            if (last_pipeline_name != "")
            {
                PixL_EndRenderPass();
            }
            if (needToClear)
            {
                PixL_StartRenderPass(layer._TextureName, "", false, true);
            }
            else
            {
                PixL_StartRenderPass(layer._TextureName, "", false, false);
            }
            render_pass_started = true;
            last_pipeline_name = effect.pipeline;
        }

        if (effect.callback)
        {
            effect.callback(effect.user_data);
        }
    }
    if (render_pass_started)
    {
        PixL_EndRenderPass();
    }

    last_pipeline_name = "";
}

void PixL_2D::compositeLayers()
{
    PixL_StartRenderPass("", "", false, true); // Start composite render pass
    // Composite all layers to final output, start with highest layer
    for (int i = 255; i >= 0; --i)
    {
        if (layers.find(i) != layers.end())
        {
            if (layers[i].is_composite)
            {
                PixL_Draw(
                    "Fullscreen", "", "", 1, 6, nullptr, {}, nullptr, {layers[i]._TextureName}, {}, nullptr);
            }
        }
    }
    PixL_EndRenderPass();
}

void PixL_2D::addDrawable(uint8_t layer_id, uint16_t z_index, const std::string &pipeline_name, PixL_2D_DrawCallback callback, void *user_data)
{
    // Check if the layer exists
    if (layers.find(layer_id) == layers.end())
    {
        return; // Layer not found
    }

    // Add drawable to the layer
    layers[layer_id].drawables.push_back({z_index, pipeline_name, callback, user_data});
}

void PixL_2D::addLayerEffect(uint8_t layer_id, uint16_t z_index, const std::string &pipeline_name, PixL_2D_EffectCallback callback, void *user_data)
{

    // Check if the layer exists
    if (layers.find(layer_id) == layers.end())
    {
        return; // Layer not found
    }

    // Add effect to the layer
    layers[layer_id].effects.push_back({z_index, pipeline_name, callback, user_data});
}

std::string PixL_2D::getLayerTextureName(uint8_t layer_id)
{
    // Check if the layer exists
    if (layers.find(layer_id) == layers.end())
    {
        return ""; // Layer not found
    }

    // Return the texture name of the layer
    return layers[layer_id]._TextureName;
}

bool PixL_2D::use_PixL_2D_Pipeline(PixL_2D_Pipeline &pipeline)
{
    // Validate the pipeline
    if (!pipeline.validate())
    {
        std::cerr << "Invalid PixL_2D pipeline!" << std::endl;
        return false;
    }

    // Clear existing layers and layers textures
    for (auto &layer : layers)
    {
        PixL_DestroyTexture(layer.second._TextureName);
    }
    layers.clear();
    drawables_order.clear();

    for (const auto &layer : pipeline.layers)
    {
        // Initialize each layer with its render target
        initLayer(layer.layer_id);
        layers[layer.layer_id].is_composite = layer.is_composite;
        layers[layer.layer_id].drawables = layer.drawables;
        layers[layer.layer_id].effects = layer.effects;
    }

    for (const auto &drawable : pipeline.drawables_order)
    {
        drawables_order.push_back(drawable);
    }

    return true;
}

bool PixL_2D::use_default_PixL_2D_Pipeline()
{
    PixL_2D_Pipeline default_pipeline = {
        "Default",
        {
            {0, {}, {}, true},
            {1, {}, {}, true},
            {2, {}, {}, true},
        },
        {{0, PixL_2D_DrawableType::PIXL_2D_DRAWABLE},
         {1, PixL_2D_DrawableType::PIXL_2D_DRAWABLE},
         {2, PixL_2D_DrawableType::PIXL_2D_DRAWABLE},
         {0, PixL_2D_DrawableType::PIXL_2D_EFFECT},
         {1, PixL_2D_DrawableType::PIXL_2D_EFFECT},
         {2, PixL_2D_DrawableType::PIXL_2D_EFFECT}},
    };

    return use_PixL_2D_Pipeline(default_pipeline);
}

void PixL_2D::Callback_WindowResized()
{
}

void PixL_2D::render()
{

    for (const auto &order : drawables_order)
    {
        if (layers.find(order.first) != layers.end())
        {
            if (order.second == PixL_2D_DrawableType::PIXL_2D_DRAWABLE)
            {
                renderLayer(order.first);
            }
            else if (order.second == PixL_2D_DrawableType::PIXL_2D_EFFECT)
            {
                applyLayerEffects(order.first);
            }
        }
    }

    compositeLayers();

    cleared_layers.clear();
    last_pipeline_name = "";

    for (auto &layer : layers)
    {
        layer.second.drawables.clear();
        layer.second.effects.clear();
    }
};

bool PixL_2D_Init()
{
    if (PixL_2D::_instance == nullptr)
    {
        PixL_2D::_instance = new PixL_2D();
        return true;
    }
    return false;
}

bool PixL_2D_Quit()
{
    if (PixL_2D::_instance != nullptr)
    {
        delete PixL_2D::_instance;
        PixL_2D::_instance = nullptr;
        return true;
    }
    return false;
}

void PixL_2D_AddDrawable(uint8_t layer_id, uint16_t z_index, const char *pipeline_name, PixL_2D_DrawCallback callback, void *user_data)
{
    if (PixL_2D::_instance == nullptr)
    {
        return; // PixL_2D not initialized
    }

    PixL_2D::_instance->addDrawable(layer_id, z_index, std::string(pipeline_name), callback, user_data);
}

void PixL_2D_AddLayerEffect(uint8_t layer_id, uint16_t z_index, const std::string &pipeline_name, PixL_2D_EffectCallback effect_callback, void *user_data)
{
    if (PixL_2D::_instance == nullptr)
    {
        return; // PixL_2D not initialized
    }

    PixL_2D::_instance->addLayerEffect(layer_id, z_index, pipeline_name, effect_callback, user_data);
}

std::string PixL_2D_GetLayerTexture(uint8_t layer_id)
{
    if (PixL_2D::_instance == nullptr)
    {
        return ""; // PixL_2D not initialized
    }

    return PixL_2D::_instance->getLayerTextureName(layer_id);
}

void PixL_2D_Render()
{
    if (PixL_2D::_instance == nullptr)
    {
        return; // PixL_2D not initialized
    }

    PixL_2D::_instance->render();
}

void PixL_2D_Callback_WindowResized()
{
}

bool PixL_2D_Pipeline::validate()
{
    // Check if the pipeline has layers
    if (layers.empty())
    {
        return false; // No layers in the pipeline
    }

    // Check if the layers are unique
    std::set<uint8_t> unique_layers;
    bool has_composite = false;
    for (const auto &layer : layers)
    {
        if (unique_layers.find(layer.layer_id) != unique_layers.end())
        {
            std::cerr << "Duplicate layer ID found: " << static_cast<int>(layer.layer_id) << std::endl;
            return false; // Duplicate layer found
        }
        has_composite |= layer.is_composite;
        unique_layers.insert(layer.layer_id);
    }

    if (!has_composite)
    {
        std::cerr << "No composite layer found in the pipeline." << std::endl;
        return false; // No composite layer found
    }

    // check if the drawables_order and effects_order are valid
    std::set<std::pair<uint8_t, PixL_2D_DrawableType>> unique_drawables_order;
    for (const auto &drawable : drawables_order)
    {
        if (unique_drawables_order.find(drawable) != unique_drawables_order.end())
        {
            std::cerr << "Duplicate drawable found in order: " << static_cast<int>(drawable.first) << std::endl;
            return false; // Duplicate drawable found in order
        }
        if (unique_layers.find(drawable.first) == unique_layers.end())
        {
            std::cerr << "Drawable layer not found in pipeline layers: " << static_cast<int>(drawable.first) << std::endl;
            return false; // Drawable layer not found in pipeline layers
        }

        unique_drawables_order.insert(drawable);
    }

    return true; // Pipeline is valid
}
