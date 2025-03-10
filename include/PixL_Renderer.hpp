#pragma once

#include <SDL3/SDL.h>
#include <SDL3_Image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <vector>

// prototypes
class PixL_Renderer;
class PixL_Drawable;
class PixL_Texture;
class PixL_Tilemap;
struct PixL_Draw_Property;
struct PixL_Draw_Command;

// enums
enum PixelDataFormat
{
    // 8 bit per channel
    PIXL_PIXEL_FORMAT_RGB,
    PIXL_PIXEL_FORMAT_BGR,
    PIXL_PIXEL_FORMAT_RGBA,
    PIXL_PIXEL_FORMAT_BGRA,
    PIXL_PIXEL_FORMAT_ARGB,
    PIXL_PIXEL_FORMAT_ABGR,
};

enum PixL_DrawableType
{
    PIXL_DRAWABLE_NONE,
    PIXL_DRAWABLE_TEXTURE,
    PIXL_DRAWABLE_TILEMAP,
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

    // flags
    uint32_t _flags;
    SDL_Window *_window = nullptr;
    SDL_Renderer *_renderer = nullptr;
    std::vector<PixL_Draw_Command *> _drawables = std::vector<PixL_Draw_Command *>();

    int Draw();
    int Present();
    // draw functions for each mode
    int DrawQueueMode();
    int DrawPlanMode();

    int draw_texture(PixL_Texture *texture, PixL_Draw_Property &property, int screenw, int screenh);
    int draw_tilemap(PixL_Tilemap *tilemap, int screenw, int screenh);
    int draw_mode7(PixL_Texture *texture, PixL_Draw_Property &property, int screenw, int screenh);

    // effects functions
    bool mods_texture(PixL_Texture *texture, PixL_Draw_Property &property);
    SDL_Texture *mosaic(PixL_Texture *texture, PixL_Draw_Property &property, int screenw, int screenh);

    // friend functions

    friend int PixL_Renderer_Init(uint32_t flags);
    friend int PixL_Renderer_Quit();
    friend SDL_Window *CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
    friend SDL_Window *GetWindow();
    friend PixL_Texture *CreateTexture(const char *path);
    friend PixL_Texture *CreateTexture(std::vector<uint8_t> &data, int width, int height, PixelDataFormat format);
    friend int PixL_Draw();
    friend int PixL_Present();
    friend uint16_t PixL_AddDrawable(PixL_Drawable *drawable, PixL_Draw_Property property);
    friend void PixL_RemoveDrawable(uint16_t id);
    friend PixL_Draw_Property *PixL_GetDrawableProperty(uint16_t id);
};

// Drawables objects classes

struct PixL_Draw_Property
{
    // plan of the texture, bigger plan will be rendered on top of smaller plan. Only used in plan mode
    uint16_t _plan = 0;

    // center of the texture in the window, between 0 and 1 (0.5 is the center of the window), bigger than 1 will make the center of the texture outside of the window
    float x = 0.5;
    float y = 0.5;

    // width and height of the texture, between 0 and 1 (1 is the full size of the window), bigger than 1 will make the texture bigger than the window
    float w = 1;
    float h = 1;

    // src rect of the texture, between 0 and 1 (1 is the full size of the texture)
    float src_x = 0.5;
    float src_y = 0.5;
    float src_w = 1;
    float src_h = 1;

    // angle of the texture in degrees, rotation is clockwise and the center of rotation is the center of the texture
    float rot = 0;
    float rot_center_x = 0.5;
    float rot_center_y = 0.5;

    // flip of the texture
    bool flip_h = false;
    bool flip_v = false;

    // color modulation
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    // mode7 properties
    bool mode7 = false;
    bool mode7_repeat = false;
    float mode7_scale = 1.0f;

    // in 2D based (so y and z are inverted compared to 3D)
    float mode7_cam_x = 0.5f;
    float mode7_cam_y = 0.5f;
    float mode7_cam_z = 1.0f;

    // mode7 rotation in degrees (x, y, z) clockwise
    float mode7_cam_rot_x = 0.0f;
    float mode7_cam_rot_y = 0.0f;
    float mode7_cam_rot_z = 0.0f;

    float mode7_focal = 1.0f;

    // mosaic properties
    uint8_t mosaic_mode = 0; // 0: no mosaic, 1-255: mosaic level (1 is the smallest level)
};

struct PixL_Draw_Command
{
    uint16_t id;
    PixL_Draw_Property property;
    PixL_Drawable *drawable;
};

class PixL_Drawable
{
protected:
    virtual ~PixL_Drawable() = default;
    virtual PixL_DrawableType getType() const = 0;

    friend PixL_Texture *CreateTexture(const char *path);
    friend PixL_Texture *CreateTexture(std::vector<uint8_t> &data, int width, int height, PixelDataFormat format);
    friend class PixL_Renderer;
    friend class PixL_Texture_Manager;
};

class PixL_Texture : public PixL_Drawable
{
protected:
    PixL_DrawableType getType() const override { return PixL_DrawableType::PIXL_DRAWABLE_TEXTURE; }
    PixL_Texture();
    ~PixL_Texture();

    uint16_t _width;
    uint16_t _height;
    SDL_Texture *_texture = nullptr;
    SDL_Texture *_texture_mod = nullptr;
    SDL_Surface *_surface = nullptr; // pour les effets gérés par cpu

    // effects parameters
    uint8_t _mosaic_mode = 0;

    friend PixL_Texture *CreateTexture(const char *path);
    friend PixL_Texture *CreateTexture(std::vector<uint8_t> &data, int width, int height, PixelDataFormat format);
    friend class PixL_Renderer;
    friend class PixL_Texture_Manager;
};

class PixL_Tilemap : public PixL_Drawable
{
protected:
    uint16_t _id;
    SDL_Texture *_texture = nullptr;
    uint16_t _GetMapHeight();
    uint16_t _GetMapWidth();
    void _SetMapHeight(uint16_t height);
    void _SetMapWidth(uint16_t width);
    std::vector<uint16_t> *_GetTilemap();

    std::vector<uint16_t> _map;
    uint16_t _map_width;
    uint16_t _map_height;
    friend PixL_Texture *CreateTexture(const char *path);
    friend PixL_Texture *CreateTexture(std::vector<uint8_t> &data, int width, int height, PixelDataFormat format);
    friend class PixL_Renderer;
    friend class PixL_Texture_Manager;
};

// function prototypes

int PixL_Renderer_Init(uint32_t flags);

int PixL_Renderer_Quit();

// Window functions
SDL_Window *CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
SDL_Window *GetWindow();

// Texture functions
PixL_Texture *CreateTexture(const char *path);
PixL_Texture *CreateTexture(std::vector<uint8_t> &data, int width, int height, PixelDataFormat format);

// Drawable functions
uint16_t PixL_AddDrawable(PixL_Drawable *drawable, PixL_Draw_Property property);
void PixL_RemoveDrawable(uint16_t id);
PixL_Draw_Property *PixL_GetDrawableProperty(uint16_t id);

// Tilemap functions

// Draw functions
int PixL_Draw();
int PixL_Present();
