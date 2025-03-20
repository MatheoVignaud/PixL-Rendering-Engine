#include <SDL_GPUAbstract.hpp>
#include <chrono>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

struct Vertex
{
	glm::vec3 position;
	glm::u8vec4 color;
};

/*struct UBOData
{
	alignas(8) glm::vec2 center = glm::vec2(0.0f, 0.0f);
	alignas(4) float w = 0.5f;
	alignas(4) float h = 0.5f;
	alignas(8) glm::vec2 src_center = glm::vec2(0.5f, 0.5f);
	alignas(4) float src_w = 1.0f;
	alignas(4) float src_h = 1.0f;
	alignas(4) float rotation = 0.0f;
	alignas(8) glm::vec2 rotation_center = glm::vec2(-1.0f, -1.0f);
	alignas(4) uint32_t flip_h = false;
	alignas(4) uint32_t flip_v = false;
	alignas(16) glm::vec4 colorMod = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
};*/

Shader_Struct VertexShader = {
	"RawTriangle.vert",
	SDL_GPU_SHADERSTAGE_VERTEX,
	0,
	0,
	0,
	0};

Shader_Struct FragmentShader = {
	"SolidColor.frag",
	SDL_GPU_SHADERSTAGE_FRAGMENT,
	1,
	0,
	0,
	0};

int main(int argc, char *argv[])
{
	SDL_SetHint(SDL_HINT_RENDER_VULKAN_DEBUG, "1");
	SDL_Init(SDL_INIT_VIDEO);
	// Get the window
	SDL_Window *window = SDL_CreateWindow("PixL", 800, 600, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
	if (device == NULL)
	{
		std::cerr << "Device could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	if (!SDL_ClaimWindowForGPUDevice(device, window))
	{
		std::cerr << "Window could not be claimed for device! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	std::cout << SDL_GetGPUDeviceDriver(device) << std::endl;

	SDL_GPUGraphicsPipeline *Pipeline = CreatePipeline(device, window, &VertexShader, &FragmentShader);

	SDL_GPUTextureSamplerBinding spriteSampler = CreateSamplerFromImage(device, "test.png");

	bool running = true;
	SDL_Event event;
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
			}
		}

		SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
		if (commandBuffer == NULL)
		{
			std::cerr << "Command buffer could not be acquired! SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}

		SDL_GPUTexture *swapchainTexture;
		if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, NULL, NULL))
		{
			SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
			return -1;
		}

		if (swapchainTexture != NULL)
		{

			SDL_GPUColorTargetInfo colorTargetInfo = {};
			colorTargetInfo.texture = swapchainTexture;
			colorTargetInfo.cycle = false;
			colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
			colorTargetInfo.clear_color = {0, 0, 0, 1};

			SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(
				commandBuffer,
				&colorTargetInfo,
				1,
				NULL);
			SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);

			SDL_BindGPUFragmentSamplers(
				renderPass,
				0,
				&spriteSampler,
				1);
			SDL_DrawGPUPrimitives(
				renderPass,
				6,
				1,
				0,
				0);

			SDL_EndGPURenderPass(renderPass);
		}
		SDL_SubmitGPUCommandBuffer(commandBuffer);
	}

	SDL_ReleaseGPUTexture(device, spriteSampler.texture);
	SDL_ReleaseGPUSampler(device, spriteSampler.sampler);
	SDL_ReleaseGPUGraphicsPipeline(device, Pipeline);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}