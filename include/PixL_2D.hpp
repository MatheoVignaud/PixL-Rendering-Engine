#pragma once

#include <PixL_Renderer.hpp>
#include <SDL_GPUAbstract.hpp>
#include <algorithm>
#include <const/generated_shaders.hpp>
#include <map>
#include <memory>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct PixL_2D_Effect PixL_2D_Effect;

    typedef void (*PixL_2D_DrawCallback)(void *user_data);

    typedef void (*PixL_2D_EffectCallback)(void *user_data);

    bool PixL_2D_Init();
    bool PixL_2D_Quit();

    void PixL_2D_AddDrawable(
        uint8_t layer_id,
        uint16_t z_index,
        const char *pipeline_name,
        PixL_2D_DrawCallback draw_callback,
        void *user_data);

    void PixL_2D_AddLayerEffect(
        uint8_t layer_id,
        uint16_t z_index,
        const std::string &pipeline_name,
        PixL_2D_EffectCallback effect_callback,
        void *user_data);

    std::string PixL_2D_GetLayerTexture(uint8_t layer_id);

    void PixL_2D_Render();

    void PixL_2D_Callback_WindowResized();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// Internal per-drawable data
struct DrawableWrapper
{
    uint16_t z_index;
    std::string pipeline;
    PixL_2D_DrawCallback callback;
    void *user_data;
};

// Layer effect information
struct LayerEffect
{
    uint16_t z_index;
    std::string pipeline;
    PixL_2D_EffectCallback callback;
    void *user_data;
};

// Layer information containing render target and drawables
struct Layer
{
    std::string _TextureName;

    // pipeline_drawables and pipeline_effects are permanent

    std::vector<DrawableWrapper> pipeline_drawables;
    std::vector<DrawableWrapper> drawables;

    std::vector<LayerEffect> pipeline_effects;
    std::vector<LayerEffect> effects;
    bool is_composite = false;
};

struct PixL_2D_Pipeline_Layer
{
    uint8_t layer_id;
    std::vector<DrawableWrapper> drawables;
    std::vector<LayerEffect> effects;
    bool is_composite = false;
};

enum class PixL_2D_DrawableType
{
    PIXL_2D_DRAWABLE,
    PIXL_2D_EFFECT
};

struct PixL_2D_Pipeline
{
    std::string name;
    std::vector<PixL_2D_Pipeline_Layer> layers;

    std::vector<std::pair<uint8_t, PixL_2D_DrawableType>> drawables_order;

    bool validate();
};

// Definition of the PixL_2D class for C++ usage
class PixL_2D
{
private:
    static PixL_2D *_instance;
    std::map<uint8_t, Layer> layers;
    std::string last_pipeline_name;
    std::vector<std::pair<uint8_t, PixL_2D_DrawableType>> drawables_order;

    std::set<uint8_t> cleared_layers;

protected:
    PixL_2D();
    ~PixL_2D();

    // Initialize layer with its render target
    void initLayer(uint8_t layer_id, uint16_t width = 800, uint16_t height = 400);

    // Render a single layer to its texture
    void renderLayer(uint8_t layer_id);

    // Apply all effects to a layer
    void applyLayerEffects(uint8_t layer_id);

    // Composite all layers to final output
    void compositeLayers();

    // Add a drawable to a specific layer
    void addDrawable(uint8_t layer_id,
                     uint16_t z_index,
                     const std::string &pipeline_name,
                     PixL_2D_DrawCallback callback,
                     void *user_data);

    // Add effect to a specific layer
    void addLayerEffect(uint8_t layer_id,
                        uint16_t z_index,
                        const std::string &pipeline_name,
                        PixL_2D_EffectCallback callback,
                        void *user_data);

    // Get texture for a layer
    std::string getLayerTextureName(uint8_t layer_id);

    bool use_PixL_2D_Pipeline(PixL_2D_Pipeline &pipeline);
    bool use_default_PixL_2D_Pipeline();

    void Callback_WindowResized();

    // Render all layers
    void render();
    //

    // C API
    friend bool PixL_2D_Init();
    friend bool PixL_2D_Quit();

    friend void PixL_2D_AddDrawable(
        uint8_t layer_id,
        uint16_t z_index,
        const char *pipeline_name,
        PixL_2D_DrawCallback callback,
        void *user_data);
    friend void PixL_2D_AddLayerEffect(
        uint8_t layer_id,
        uint16_t z_index,
        const std::string &pipeline_name,
        PixL_2D_EffectCallback effect_callback,
        void *user_data);
    friend std::string PixL_2D_GetLayerTexture(uint8_t layer_id);
    friend void PixL_2D_Render();
    friend void PixL_2D_Callback_WindowResized();
};

#endif // __cplusplus
