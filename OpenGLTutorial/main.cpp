#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

#include "vertex_buffer.h"
#include "index_buffer.h"
#include "shader.h"

#pragma region int main(int argc, char** argv)
#ifdef _DEBUG
int main(int argc, char** argv) {
#else
//extern "C" int WinMain(int argc, char** argv) {
#include <Windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow){
#endif // _DEBUG
#pragma endregion
	SDL_Window* window;
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	uint32_t flags = SDL_WINDOW_OPENGL;
#pragma region Release window flags
#ifndef _DEBUG
	flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif // !_DEBUG
#pragma endregion

	window = SDL_CreateWindow("OpenGL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	Vertex vertices[] = {
		Vertex{-0.5, -0.5, 0, 1, 0, 0, 1},
		Vertex{-0.5, 0.5, 0, 0, 1, 0, 1},
		Vertex{0.5, -0.5, 0, 0, 0, 1, 1},
		Vertex{0.5, 0.5, 0, 1, 0, 0, 1},
	};
	uint32_t numVertices = 4;

	uint32_t indices[] = {
		0, 1, 2,
		1, 2, 3
	};
	uint32_t numIndices = 6;

	IndexBuffer indexBuffer(indices, numIndices, sizeof(uint32_t));

	VertexBuffer vertexBuffer(vertices, numVertices);

	Shader shader("basic.vert", "basic.frag");
	shader.bind();
	
	uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64_t lastCounter = SDL_GetPerformanceCounter() ;
	float delta = 0;

	bool close = false;
	while (!close)
	{
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// wire frame mode
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		vertexBuffer.bind();
		indexBuffer.bind();
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) {
				close = true;
			}
		}

		uint64_t endCounter = SDL_GetPerformanceCounter();
		uint64_t counterElapse = endCounter - lastCounter;
		delta = (float)counterElapse / (float)perfCounterFrequency;
		uint32_t FPS = (uint32_t)((float)perfCounterFrequency / (float)counterElapse);
		std::cout << "FPS: " << FPS << std::endl;
		lastCounter = endCounter;
	}
	return 0;
}