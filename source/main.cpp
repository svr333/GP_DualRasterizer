#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"
#include "main.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Dual Rasterizer - Senne Van Rompaey GD08",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	bool shouldPrint = false;
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
					pRenderer->ToggleRasterizerMode();
				else if (e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleMeshRotation();
				else if (e.key.keysym.scancode == SDL_SCANCODE_F9)
					pRenderer->CycleCullingMode();
				else if (e.key.keysym.scancode == SDL_SCANCODE_F10)
					pRenderer->ToggleUniformColor();
				else if (e.key.keysym.scancode == SDL_SCANCODE_F11)
					shouldPrint = !shouldPrint;

				// only allow specific shortcuts if in correct render mode
				if (pRenderer->GetRenderMode() == Renderer::RenderMode::Hardware)
				{
					if (e.key.keysym.scancode == SDL_SCANCODE_F3)
						pRenderer->GetHardwareRenderer()->ToggleFireFxMesh();
					else if (e.key.keysym.scancode == SDL_SCANCODE_F4)
						pRenderer->CycleSampleState();
				}
				else if (pRenderer->GetRenderMode() == Renderer::RenderMode::Software)
				{
					if (e.key.keysym.scancode == SDL_SCANCODE_F5)
						pRenderer->GetSoftwareRenderer()->CycleLightingMode();
					else if (e.key.keysym.scancode == SDL_SCANCODE_F6)
						pRenderer->GetSoftwareRenderer()->ToggleNormalMap();
					else if (e.key.keysym.scancode == SDL_SCANCODE_F7)
						pRenderer->GetSoftwareRenderer()->ToggleDepthBufferVisualization();
					else if (e.key.keysym.scancode == SDL_SCANCODE_F8)
						pRenderer->GetSoftwareRenderer()->ToggleBoundingBoxVisualization();
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f && shouldPrint)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}