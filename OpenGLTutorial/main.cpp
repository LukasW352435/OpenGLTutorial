#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

#include "vertex_buffer.h"
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

	window = SDL_CreateWindow("OpenGL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	Vertex vertices[] = {
		Vertex{-0.5, -0.5, 0},
		Vertex{0, 0.5, 0},
		Vertex{0.5, -0.5, 0},
	};

	uint32_t numVertices = 3;

	VertexBuffer vertexBuffer(vertices, numVertices);

	Shader shader("basic.vert", "basic.frag");
	shader.bind();
	
	bool close = false;
	while (!close)
	{
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		vertexBuffer.bind();
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
		vertexBuffer.unbind();

		SDL_GL_SwapWindow(window);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) {
				close = true;
			}
		}
	}
	return 0;
}