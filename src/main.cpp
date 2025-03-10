#include <PixL_Renderer.hpp>
#include <chrono>
#include <glm/glm.hpp>
#include <iostream>
#include <thread>
static const char *BasePath = SDL_GetBasePath();

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
	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
	const char *entrypoint;

	if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
	{
		SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/SPIRV/%s.spv", BasePath, shaderFilename);
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	}
	else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
	{
		SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/MSL/%s.msl", BasePath, shaderFilename);
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	}
	else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
	{
		SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/DXIL/%s.dxil", BasePath, shaderFilename);
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	}
	else
	{
		SDL_Log("%s", "Unrecognized backend shader format!");
		return NULL;
	}

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

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	// Get the window
	SDL_Window *window = SDL_CreateWindow("PixL", 800, 600, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
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

	SDL_GPUShader *vert = LoadShader(device, "PositionColor.vert", 0, 0, 0, 0);
	if (vert == NULL)
	{
		std::cerr << "Shader could not be loaded! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	SDL_GPUShader *frag = LoadShader(device, "SolidColor.frag", 0, 0, 0, 0);

	SDL_GPUColorTargetDescription colorTargetDescription{};
	colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device, window);
	std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions{colorTargetDescription};

	SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
	targetInfo.color_target_descriptions = colorTargetDescriptions.data();
	targetInfo.num_color_targets = colorTargetDescriptions.size();

	std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
	std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions{};

	vertexAttributes.emplace_back(SDL_GPUVertexAttribute{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0});
	vertexAttributes.emplace_back(SDL_GPUVertexAttribute{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, sizeof(float) * 3});

	vertexBufferDescriptions.emplace_back(0, sizeof(glm::vec3), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);

	SDL_GPUVertexInputState vertexInputState{};
	vertexInputState.vertex_attributes = vertexAttributes.data();
	vertexInputState.num_vertex_attributes = vertexAttributes.size();
	vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions.data();
	vertexInputState.num_vertex_buffers = vertexBufferDescriptions.size();

	SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.vertex_shader = vert;
	pipelineInfo.fragment_shader = frag;
	pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	pipelineInfo.target_info = targetInfo;
	pipelineInfo.vertex_input_state = vertexInputState;

	SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
	if (pipeline == NULL)
	{
		std::cerr << "Pipeline could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_ReleaseGPUShader(device, vert);
	SDL_ReleaseGPUShader(device, frag);

	std::vector<Vertex> vertices{
		{{0.0f, 0.5f, 0.0f}, {255, 0, 0, 255}},
		{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}},
		{{-0.5f, -0.5f, 0.0f}, {0, 0, 255, 255}}};

	SDL_GPUBufferCreateInfo bufferInfo{};
	bufferInfo.size = sizeof(Vertex) * vertices.size();
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

	SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);
	if (vertexBuffer == NULL)
	{
		std::cerr << "Buffer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_GPUBufferCreateInfo transferBufferInfo{};
	transferBufferInfo.size = bufferInfo.size;
	transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	SDL_GPUBuffer *transferBuffer = SDL_CreateGPUBuffer(device, &transferBufferInfo);
	if (transferBuffer == NULL)
	{
		std::cerr << "Buffer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	std::span<

	SDL_ShowWindow(window);

	bool quit = false;
	SDL_Event event;
	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				quit = true;
				break;

			default:
				break;
			}
		}
		SDL_GPUCommandBuffer *buffer{SDL_AcquireGPUCommandBuffer(device)};
		if (buffer == NULL)
		{
			std::cerr << "Buffer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}

		SDL_GPUTexture *swapchainTexture;
		SDL_WaitAndAcquireGPUSwapchainTexture(buffer, window, &swapchainTexture, nullptr, nullptr);
		if (swapchainTexture)
		{
			SDL_GPUColorTargetInfo colorTarget{};
			colorTarget.texture = swapchainTexture;
			colorTarget.store_op = SDL_GPU_STOREOP_STORE;
			colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
			colorTarget.clear_color = SDL_FColor{0.1f, 0.1f, 0.1f, 1.0f};
			std::vector<SDL_GPUColorTargetInfo> colorTargets{colorTarget};
			SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(buffer, colorTargets.data(), colorTargets.size(), nullptr);
			SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
			SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
			SDL_EndGPURenderPass(renderPass);
		}
		if (!SDL_SubmitGPUCommandBuffer(buffer))
		{
			std::cerr << "Buffer could not be submitted! SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}
	}

	return 0;
}