#include <iostream>
#include <vector>
#include <fstream>
#define GLEW_STATIC
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "../dependencies/glm/glm.hpp"
#include "../dependencies/glm/ext/matrix_transform.hpp"
#include "../dependencies/glm/gtc/matrix_transform.hpp"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "shader.h"
#include "mesh.h"
#include "floating_camera.h"

#define MONKEY_FILE "C:\\Users\\Lukas\\source\\repos\\OpenGLTutorial\\models\\monkey.bmf"
#define TREE_FILE "C:\\Users\\Lukas\\source\\repos\\OpenGLTutorial\\models\\tree01.bmf"

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
	// 0 immediate updates
	// 1 updates synchronized with the vertical retrace
	// -1 adaptive vsync
	SDL_GL_SetSwapInterval(-1);
	SDL_SetRelativeMouseMode(SDL_TRUE);

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
	
	int32_t textureWidth = 0;
	int32_t textureHeight = 0;
	int32_t bitsPerPixel = 0;
	stbi_set_flip_vertically_on_load(true);
	auto textureBuffer = stbi_load("yellow.png", &textureWidth, &textureHeight, &bitsPerPixel, 4);
	
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

	Model monkey;
	monkey.Init(MONKEY_FILE, &shader);
	
	uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64_t lastCounter = SDL_GetPerformanceCounter() ;
	float delta = 0;

	glm::mat4 model = glm::mat4(1);
	model = glm::scale(model, glm::vec3(1.0f));

	FloatingCamera camera(90, 800.0f, 600.0f);
	camera.translate(glm::vec3(0.0f, 0.0f, 5.0f));
	camera.update();

	glm::mat4 modelViewProj = camera.getViewProj() * model;
	int modelViewProjMatrixLocation = glGetUniformLocation(shader.getShaderId(), "u_modelViewProj");
	int modelViewLocation = glGetUniformLocation(shader.getShaderId(), "u_modelView");
	int invModelViewLocation = glGetUniformLocation(shader.getShaderId(), "u_invModelView");

	float time = 0;
	float cameraSpeed = 6.0f;
	bool close = false;
	bool buttonW = false;
	bool buttonS = false;
	bool buttonA = false;
	bool buttonD = false;
	bool buttonSpace = false;
	bool buttonShift = false;
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	while (!close)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) {
				close = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					close = true;
				}
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					buttonW = true;
					break;
				case SDLK_a:
					buttonA = true;
					break;
				case SDLK_s:
					buttonS = true;
					break;
				case SDLK_d:
					buttonD = true;
					break;
				case SDLK_SPACE:
					buttonSpace = true;
					break;
				case SDLK_LSHIFT:
					buttonShift = true;
					break;
				case SDLK_TAB:
					SDL_SetRelativeMouseMode(SDL_FALSE);
					break;
				default:
					break;
				}
			}
			else if (event.type == SDL_KEYUP) {
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					buttonW = false;
					break;
				case SDLK_a:
					buttonA = false;
					break;
				case SDLK_s:
					buttonS = false;
					break;
				case SDLK_d:
					buttonD = false;
					break;
				case SDLK_SPACE:
					buttonSpace = false;
					break;
				case SDLK_LSHIFT:
					buttonShift = false;
					break;
				default:
					break;
				}
			}
			else if (event.type == SDL_MOUSEMOTION) {
				if (SDL_GetRelativeMouseMode()) {
					camera.onMouseMoved(event.motion.xrel, event.motion.yrel);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
			}
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		time += delta;

		if (buttonW) {
			camera.moveFront(cameraSpeed * delta);
		}
		if (buttonA) {
			camera.moveSideways(-cameraSpeed * delta);
		}
		if (buttonS) {
			camera.moveFront(-cameraSpeed * delta);
		}
		if (buttonD) {
			camera.moveSideways(cameraSpeed * delta);
		}
		if (buttonShift) {
			camera.moveUp(-cameraSpeed* delta);
		}
		if (buttonSpace) {
			camera.moveUp(cameraSpeed * delta);
		}
		camera.update();

		model = glm::rotate(model, 1 * delta, glm::vec3(0.0f, 1.0f, 0.0f));
		modelViewProj = camera.getViewProj() * model;
		glm::mat4 modelView = camera.getView() * model;
		glm::mat4 invModelView = glm::transpose(glm::inverse(modelView));

		// wire frame mode
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (modelViewLocation != -1) {
			glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, &modelView[0][0]);
		}
		if (invModelViewLocation != -1) {
			glUniformMatrix4fv(invModelViewLocation, 1, GL_FALSE, &invModelView[0][0]);
		}
		if (modelViewProjMatrixLocation != -1) {
			glUniformMatrix4fv(modelViewProjMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		monkey.render();
		SDL_GL_SwapWindow(window);

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