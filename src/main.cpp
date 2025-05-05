#include <PixL_2D.hpp>
#include <PixL_Renderer.hpp>
#include <chrono>

int main(int argc, char *argv[])
{
	PixL_Renderer_Init(0);
	SDL_Window *window = CreateWindow("PixL Renderer", 800, 600, SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		SDL_Log("Could not create window: %s", SDL_GetError());
		return -1;
	}

	PixL_2D_Init();

	PixL_CreateTexture("test_texture", "test.png");

	auto start = std::chrono::high_resolution_clock::now();

	PixL_Callback_WindowResized();

	PixL_CreateVBO("default", sizeof(glm::vec2) * 6);

	bool quit = false;
	while (!quit)
	{

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				quit = true;
			}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED)
			{
				PixL_Callback_WindowResized();
			}
		}

		PixL_StartDraw();

		for (int i = 0; i < 2; i++)
		{
			// create random vertex data
			// between -1 and 1

			std::vector<glm::vec2> vertices = {
				glm::vec2(
					(float)(rand() % 2000) / 1000.0f - 1.0f,
					(float)(rand() % 2000) / 1000.0f - 1.0f),
				glm::vec2(
					(float)(rand() % 2000) / 1000.0f - 1.0f,
					(float)(rand() % 2000) / 1000.0f - 1.0f),
				glm::vec2(
					(float)(rand() % 2000) / 1000.0f - 1.0f,
					(float)(rand() % 2000) / 1000.0f - 1.0f),
				glm::vec2(
					(float)(rand() % 2000) / 1000.0f - 1.0f,
					(float)(rand() % 2000) / 1000.0f - 1.0f),
				glm::vec2(
					(float)(rand() % 2000) / 1000.0f - 1.0f,
					(float)(rand() % 2000) / 1000.0f - 1.0f),
				glm::vec2(
					(float)(rand() % 2000) / 1000.0f - 1.0f,
					(float)(rand() % 2000) / 1000.0f - 1.0f)};
		}

		PixL_SwapBuffers();

		// print time taken and fps
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		start = end;
		double fps = 1000.0 / elapsed.count();
		std::cout << "FPS: " << fps << std::endl;
		std::cout << "Time: " << elapsed.count() << "ms" << std::endl;
		std::cout << "Draw Calls: " << PixL_GetDrawCalls() << std::endl;
		std::cout << "------------------------" << std::endl;
	}

	PixL_Renderer_Quit();

	return 0;
}