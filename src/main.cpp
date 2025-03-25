#include <SDL_GPUAbstract.hpp>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
	.shader_Path = "RawTriangle.vert",
	.shader_Stage = SDL_GPU_SHADERSTAGE_VERTEX,
	.sampler_Count = 0,
	.uniform_Buffer_Count = 1,
	.storage_Buffer_Count = 0,
	.storage_Texture_Count = 0,
	.vertexAttributes = {
		(SDL_GPUVertexAttribute){
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0,
		},
		(SDL_GPUVertexAttribute){
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = sizeof(float) * 3,
		},
	},
	.vertexBuffers = {
		(SDL_GPUVertexBufferDescription){
			.slot = 0,
			.pitch = sizeof(float) * 5,
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0,
		},
	},

};

Shader_Struct FragmentShader = {
	"SolidColor.frag",
	SDL_GPU_SHADERSTAGE_FRAGMENT,
	2,
	0,
	0,
	0};

Shader_Struct VertexShader2 = {
	.shader_Path = "DepthViewer.vert",
	.shader_Stage = SDL_GPU_SHADERSTAGE_VERTEX,
	.sampler_Count = 0,
	.uniform_Buffer_Count = 0,
	.storage_Buffer_Count = 0,
	.storage_Texture_Count = 0,

};

Shader_Struct FragmentShader2 = {
	"DepthViewer.frag",
	SDL_GPU_SHADERSTAGE_FRAGMENT,
	1,
	0,
	0,
	0};

void camDir_callback(SDL_Event event, float &pitch, float &yaw)
{
	const float camSensitivity = 0.2f;
	if (event.motion.y > 0 && event.motion.y < 600 && event.motion.yrel > -100 && event.motion.yrel < 100)

		pitch += event.motion.yrel * camSensitivity;

	if (event.motion.x > 0 && event.motion.x < 800 && event.motion.xrel > -100 && event.motion.xrel < 100)

		yaw += event.motion.xrel * camSensitivity;
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

	SDL_GPUGraphicsPipeline *Pipeline = CreatePipeline(device, window, &VertexShader, &FragmentShader, true);
	SDL_GPUGraphicsPipeline *PipelineZBufferViwer = CreatePipeline(device, window, &VertexShader2, &FragmentShader2, false);

	SDL_GPUTextureSamplerBinding spriteSampler = CreateSamplerFromImage(device, "test.bmp");
	SDL_GPUTextureSamplerBinding ravioli = CreateSamplerFromImage(device, "ravioli_atlas.bmp");

	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

	VertexBuffer_Struct vertexBuffer = CreateVBO(device, sizeof(vertices));
	vertexBuffer.Update(device, vertices, sizeof(vertices));
	vertexBuffer.Upload(device);

	struct MVP
	{
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
	};

	float aspect = 800.0f / 600.0f;
	auto start = std::chrono::high_resolution_clock::now();
	// get seconds since epoch
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0f;

	MVP mvp;
	mvp.model = glm::rotate(mvp.model, time * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
	mvp.view = glm::translate(mvp.view, glm::vec3(0.0f, 0.0f, -3.0f));
	mvp.projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 100.0f);
	TransferBuffer_Struct ubo = CreateUBO(device, sizeof(MVP));

	DepthBuffer_Struct *depthBuffer = CreateDepthBuffer(device, 800, 600);

	glm::vec3 cubePositions[] = {
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f)};

	// camera
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
	const float cameraSpeed = 0.005f;

	float lastX = 400, lastY = 300;
	float yaw = -90;
	float pitch = -180;

	bool running = true;
	bool showDepthBuffer = false;
	SDL_Event event;
	auto delta_time_chrono = std::chrono::high_resolution_clock::now();

	bool first_frame = true;

	int first_frames_counter = 200;

	while (running)
	{
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0f;
		float angle = 20.0f * time;

		glm::mat4 model = glm::mat4(1.0f);

		float delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - delta_time_chrono).count();
		delta_time_chrono = std::chrono::high_resolution_clock::now();

		float cameraSpeedDelta = cameraSpeed * delta_time;

		float xoffset = 0, yoffset = 0;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				if (event.key.key == SDLK_D)
				{
					showDepthBuffer = !showDepthBuffer;
				}
				if (event.key.key == SDLK_UP)
				{
					cameraPos -= cameraSpeedDelta * cameraFront;
				}
				if (event.key.key == SDLK_DOWN)
				{
					cameraPos += cameraSpeedDelta * cameraFront;
				}
				if (event.key.key == SDLK_LEFT)
				{
					cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeedDelta;
				}
				if (event.key.key == SDLK_RIGHT)
				{
					cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeedDelta;
				}
			}
			if ((event.motion.yrel || event.motion.yrel) && !first_frame)
			{
				camDir_callback(event, pitch, yaw);
			}
		}

		std::cout << "yaw: " << yaw << " pitch: " << pitch << std::endl;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		cameraFront = glm::normalize(direction);

		mvp.view = glm::lookAt(cameraPos, cameraPos - cameraFront, cameraUp);

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

			SDL_GPUColorTargetInfo colorTargetInfo = {0};
			colorTargetInfo.texture = swapchainTexture;
			colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
			colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

			SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(
				commandBuffer,
				&colorTargetInfo,
				1,
				&depthBuffer->depthStencilTargetInfoClear);

			mvp.model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
			mvp.model = glm::rotate(mvp.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

			SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
			vertexBuffer.Bind(renderPass, 0);
			SDL_BindGPUFragmentSamplers(renderPass, 0, &spriteSampler, 1);
			SDL_BindGPUFragmentSamplers(renderPass, 1, &ravioli, 1);
			SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(MVP));
			SDL_DrawGPUPrimitives(
				renderPass,
				36,
				1,
				0,
				0);

			SDL_EndGPURenderPass(renderPass);

			colorTargetInfo = {0};
			colorTargetInfo.texture = swapchainTexture;
			colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
			colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
			colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

			for (int i = 0; i < 9; i++)
			{
				mvp.model = glm::translate(model, cubePositions[i]);
				mvp.model = glm::rotate(mvp.model, glm::radians(angle + i * 4), glm::vec3(1.0f, 0.3f, 0.5f));

				renderPass = SDL_BeginGPURenderPass(
					commandBuffer,
					&colorTargetInfo,
					1,
					&depthBuffer->depthStencilTargetInfoLoad);
				SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
				vertexBuffer.Bind(renderPass, 0);
				SDL_BindGPUFragmentSamplers(renderPass, 0, &spriteSampler, 1);
				SDL_BindGPUFragmentSamplers(renderPass, 1, &ravioli, 1);
				SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(MVP));
				SDL_DrawGPUPrimitives(
					renderPass,
					36,
					1,
					0,
					0);

				SDL_EndGPURenderPass(renderPass);
			}

			if (showDepthBuffer)
			{
				renderPass = SDL_BeginGPURenderPass(
					commandBuffer,
					&colorTargetInfo,
					1,
					NULL);
				SDL_BindGPUGraphicsPipeline(renderPass, PipelineZBufferViwer);
				SDL_BindGPUFragmentSamplers(renderPass, 0, &depthBuffer->sampler, 1);
				SDL_DrawGPUPrimitives(
					renderPass,
					6,
					1,
					0,
					0);

				SDL_EndGPURenderPass(renderPass);
			}
		}
		SDL_SubmitGPUCommandBuffer(commandBuffer);

		if (first_frames_counter < 0)
		{
			first_frames_counter--;
		}
		else
		{
			first_frame = false;
		}
	}

	ubo.Destroy(device);
	depthBuffer->Destroy(device);
	vertexBuffer.Destroy(device);
	SDL_ReleaseGPUTexture(device, depthBuffer->texture);
	SDL_ReleaseGPUSampler(device, depthBuffer->sampler.sampler);
	SDL_ReleaseGPUTexture(device, spriteSampler.texture);
	SDL_ReleaseGPUSampler(device, spriteSampler.sampler);
	SDL_ReleaseGPUTexture(device, ravioli.texture);
	SDL_ReleaseGPUSampler(device, ravioli.sampler);
	SDL_ReleaseGPUGraphicsPipeline(device, Pipeline);
	SDL_ReleaseGPUGraphicsPipeline(device, PipelineZBufferViwer);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}