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

	PixL_2D_Init(0);

	PixL_CreateTexture("test_texture", "test.png");

	auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::pair<Quad_UBO, int>> quads;

	for (int i = 0; i < 50000; i++)
	{
		glm::vec2 random_pos = {};
		// random pos between -1 and 1
		random_pos.x = (float)(rand() % 2000 - 1000) / 1000.0f;
		random_pos.y = (float)(rand() % 2000 - 1000) / 1000.0f;
		glm::vec2 random_size = {};
		// random size between 0.1 and 0.6
		random_size.x = (float)(rand() % 500) / 1000.0f + 0.1f;
		random_size.y = (float)(rand() % 500) / 1000.0f + 0.1f;
		Quad_UBO quad = {random_pos, random_size};
		quads.push_back(std::make_pair(quad, i));
	}

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

		PixL_2D_DrawTexturedQuadBatch("test_texture", quads);

		// print time taken and fps
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		start = end;
		double fps = 1000.0 / elapsed.count();
		std::cout << "FPS: " << fps << std::endl;
		std::cout << "Time: " << elapsed.count() << "ms" << std::endl;
		std::cout << "Draw Calls: " << PixL_GetDrawCalls() << std::endl;
		std::cout << "------------------------" << std::endl;

		PixL_SwapBuffers();
	}

	return 0;
}