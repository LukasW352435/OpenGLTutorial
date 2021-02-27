#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vertex_buffer.h"
#include "index_buffer.h"
#include "shader.h"

void GLAPIENTRY openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParm) {
	std::cout << "[OpenGL Error] " << message << std::endl;
}

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

#ifdef _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif // _DEBUG

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

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLDebugCallback, nullptr);
#endif // _DEBUG

	Vertex vertices[] = {
		Vertex{-0.5, -0.5, 0, 
		0, 0, 
		1, 0, 0, 1},
		Vertex{-0.5, 0.5, 0,
		0, 1,
		0, 1, 0, 1},
		Vertex{0.5, -0.5, 0, 
		1, 0,
		0, 0, 1, 1},
		Vertex{0.5, 0.5, 0, 
		1, 1,
		1, 0, 0, 1},
	};
	uint32_t numVertices = 4;

	uint32_t indices[] = {
		0, 1, 2,
		1, 2, 3
	};
	uint32_t numIndices = 6;

	IndexBuffer indexBuffer(indices, numIndices, sizeof(uint32_t));

	VertexBuffer vertexBuffer(vertices, numVertices);

	int32_t textureWidth = 0;
	int32_t textureHeight = 0;
	int32_t bitsPerPixel = 0;
	stbi_set_flip_vertically_on_load(true);
	auto textureBuffer = stbi_load("redSmoke.png", &textureWidth, &textureHeight, &bitsPerPixel, 4);
	
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (textureBuffer) {
		stbi_image_free(textureBuffer);
	}

	Shader shader("basic.vert", "basic.frag");
	shader.bind();
	
	uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64_t lastCounter = SDL_GetPerformanceCounter() ;
	float delta = 0;

	int colorUniformLocation = glGetUniformLocation(shader.getShaderId(), "u_color");
	if (colorUniformLocation != -1) {
		glUniform4f(colorUniformLocation, 1, 0, 1, 1);
	}

	int textureUniformLocation = glGetUniformLocation(shader.getShaderId(), "u_texture");
	if (textureUniformLocation != -1) {
		glUniform1i(textureUniformLocation, 0);
	}

	float time = 0;
	bool close = false;
	while (!close)
	{
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		time += delta;

		if (colorUniformLocation != -1) {
			glUniform4f(colorUniformLocation, sinf(time)*sinf(time), 1, 1, 1);
		}

		// wire frame mode
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		vertexBuffer.bind();
		indexBuffer.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
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
		//std::cout << "FPS: " << FPS << std::endl;
		lastCounter = endCounter;
	}

	glDeleteTextures(1, &textureId);

	return 0;
}