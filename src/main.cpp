#include <PixL_Renderer.hpp>
#include <const/generated_shaders.hpp>

int main(int argc, char *argv[])
{
	PixL_Renderer_Init(0);
	SDL_Window *window = CreateWindow("PixL Renderer", 800, 600, SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		SDL_Log("Could not create window: %s", SDL_GetError());
		return -1;
	}

	PixL_CreateTexture("test_texture", "test.png");

	PixL_CreatePipeline("test_pipeline", &test_vertex, &test_fragment, false, SDL_GPU_COMPAREOP_LESS, false, false);

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
		PixL_Draw(
			"test_pipeline",
			"",
			"",
			1,

			6,
			nullptr,
			{nullptr, 0},
			nullptr,

			{"test_texture"},
			{nullptr, 0},
			nullptr);

		PixL_SwapBuffers();
	}

	return 0;
}