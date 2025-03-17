#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <chrono>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

static const char *BasePath = SDL_GetBasePath();

SDL_Surface *LoadImage(const char *imageFilename, int desiredChannels)
{
	char fullPath[256];
	SDL_Surface *result;
	SDL_PixelFormat format;

	SDL_snprintf(fullPath, sizeof(fullPath), "%s%s", BasePath, imageFilename);

	result = SDL_LoadBMP(fullPath);
	if (result == NULL)
	{
		SDL_Log("Failed to load BMP: %s", SDL_GetError());
		return NULL;
	}

	if (desiredChannels == 4)
	{
		format = SDL_PIXELFORMAT_ABGR8888;
	}
	else
	{
		SDL_assert(!"Unexpected desiredChannels");
		SDL_DestroySurface(result);
		return NULL;
	}
	if (result->format != format)
	{
		SDL_Surface *next = SDL_ConvertSurface(result, format);
		SDL_DestroySurface(result);
		result = next;
	}

	return result;
}

SDL_GPUShader *LoadShader(
	SDL_GPUDevice *device,
	const char *shaderFilename,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount)
{
	// Auto-detect the shader stage from the file name for convenience
	SDL_GPUShaderStage stage;
	if (SDL_strstr(shaderFilename, ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(shaderFilename, ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage!");
		return NULL;
	}

	char fullPath[256];
	const char *entrypoint;

	SDL_snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.spv", BasePath, shaderFilename);
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
	shaderInfo.stage = stage;
	shaderInfo.num_samplers = samplerCount;
	shaderInfo.num_uniform_buffers = uniformBufferCount;
	shaderInfo.num_storage_buffers = storageBufferCount;
	shaderInfo.num_storage_textures = storageTextureCount;

	SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_free(code);
		return NULL;
	}

	SDL_free(code);
	return shader;
}

struct Vertex
{
	glm::vec3 position;
	glm::u8vec4 color;
};

struct UBOData
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
};

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

	SDL_GPUShader *vertexShader = LoadShader(device, "RawTriangle.vert", 0, 1, 0, 0);
	if (vertexShader == NULL)
	{
		std::cerr << "Shader could not be loaded! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	std::cout << "Shaders loaded!" << std::endl;

	SDL_GPUShader *fragmentShader = LoadShader(device, "SolidColor.frag", 1, 0, 0, 0);
	if (fragmentShader == NULL)
	{
		std::cerr << "Shader could not be loaded! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	std::cout << "Shaders loaded!" << std::endl;

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

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.target_info = pipeline_target_info;
	pipelineCreateInfo.vertex_shader = vertexShader;
	pipelineCreateInfo.fragment_shader = fragmentShader;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

	SDL_GPUGraphicsPipeline *Pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);
	if (Pipeline == NULL)
	{
		std::cerr << "Pipeline could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	std::cout << "Pipeline created!" << std::endl;

	std::cout << "Release Shaders" << std::endl;
	SDL_ReleaseGPUShader(device, vertexShader);
	SDL_ReleaseGPUShader(device, fragmentShader);

	SDL_Surface *spriteSurface = LoadImage("test.bmp", 4);
	if (spriteSurface == NULL)
	{
		SDL_Log("Could not load image data!");
		return -1;
	}

	SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = spriteSurface->w * spriteSurface->h * 4};

	SDL_GPUTransferBuffer *textureTransferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);
	if (textureTransferBuffer == NULL)
	{
		SDL_Log("Could not create transfer buffer!");
		return -1;
	}

	void *textureTransferPtr = SDL_MapGPUTransferBuffer(
		device,
		textureTransferBuffer,
		false);

	SDL_memcpy(textureTransferPtr, spriteSurface->pixels, spriteSurface->w * spriteSurface->h * 4);
	SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

	SDL_GPUTextureCreateInfo textureCreateInfo = {};
	textureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	textureCreateInfo.width = spriteSurface->w;
	textureCreateInfo.height = spriteSurface->h;
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

	SDL_GPUTransferBufferCreateInfo uboTransferBufferCreateInfo = {};
	uboTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	uboTransferBufferCreateInfo.size = sizeof(UBOData);

	SDL_GPUTransferBuffer *uboTransferBuffer = SDL_CreateGPUTransferBuffer(
		device,
		&uboTransferBufferCreateInfo);

	SDL_GPUBufferCreateInfo uboBufferCreateInfo = {};
	uboBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	uboBufferCreateInfo.size = sizeof(UBOData);

	SDL_GPUBuffer *UBOBuffer = SDL_CreateGPUBuffer(
		device,
		&uboBufferCreateInfo);

	SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_GPUTextureTransferInfo transferInfo = {};
	transferInfo.transfer_buffer = textureTransferBuffer;
	transferInfo.offset = 0;

	SDL_GPUTextureRegion region = {};
	region.texture = Texture;
	region.w = spriteSurface->w;
	region.h = spriteSurface->h;
	region.d = 1;

	SDL_UploadToGPUTexture(
		copyPass,
		&transferInfo,
		&region,
		false);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

	SDL_DestroySurface(spriteSurface);
	SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);

	UBOData uboData;
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
				if (event.key.key == SDLK_LEFT)
				{
					uboData.center.x -= 0.1f;
				}
				if (event.key.key == SDLK_RIGHT)
				{
					uboData.center.x += 0.1f;
				}
				if (event.key.key == SDLK_UP)
				{
					uboData.center.y += 0.1f;
				}
				if (event.key.key == SDLK_DOWN)
				{
					uboData.center.y -= 0.1f;
				}
				if (event.key.key == SDLK_Z)
				{
					uboData.h += 0.1f;
				}
				if (event.key.key == SDLK_S)
				{
					uboData.h -= 0.1f;
				}
				if (event.key.key == SDLK_Q)
				{
					uboData.w -= 0.1f;
				}
				if (event.key.key == SDLK_D)
				{
					uboData.w += 0.1f;
				}
				if (event.key.key == SDLK_W)
				{
					uboData.rotation -= 0.1f;
					std::cout << "Rotation: " << uboData.rotation << std::endl;
				}
				if (event.key.key == SDLK_X)
				{
					uboData.rotation += 0.1f;
					std::cout << "Rotation: " << uboData.rotation << std::endl;
				}
				if (event.key.key == SDLK_O)
				{
					uboData.src_h += 0.1f;
				}
				if (event.key.key == SDLK_L)
				{
					uboData.src_h -= 0.1f;
				}
				if (event.key.key == SDLK_K)
				{
					uboData.src_w -= 0.1f;
				}
				if (event.key.key == SDLK_M)
				{
					uboData.src_w += 0.1f;
				}
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
			void *dataPtr = SDL_MapGPUTransferBuffer(
				device,
				uboTransferBuffer,
				true);

			SDL_UnmapGPUTransferBuffer(device, uboTransferBuffer);

			SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
			SDL_GPUTransferBufferLocation UploadUbotoGPUBuffer = {};
			UploadUbotoGPUBuffer.transfer_buffer = uboTransferBuffer;
			UploadUbotoGPUBuffer.offset = 0;

			SDL_GPUBufferRegion uboRegion = {};
			uboRegion.buffer = UBOBuffer;
			uboRegion.offset = 0;
			uboRegion.size = sizeof(UBOData);

			SDL_UploadToGPUBuffer(
				copyPass,
				&UploadUbotoGPUBuffer,
				&uboRegion,
				true);
			SDL_EndGPUCopyPass(copyPass);

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

			SDL_GPUTextureSamplerBinding textureBinding = {};
			textureBinding.texture = Texture;
			textureBinding.sampler = Sampler;

			SDL_BindGPUFragmentSamplers(
				renderPass,
				0,
				&textureBinding,
				1);
			SDL_PushGPUVertexUniformData(
				commandBuffer,
				0,
				&uboData,
				sizeof(UBOData));
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

	SDL_ReleaseGPUBuffer(device, UBOBuffer);
	SDL_ReleaseGPUTransferBuffer(device, uboTransferBuffer);
	SDL_ReleaseGPUTexture(device, Texture);
	SDL_ReleaseGPUSampler(device, Sampler);
	SDL_ReleaseGPUGraphicsPipeline(device, Pipeline);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}