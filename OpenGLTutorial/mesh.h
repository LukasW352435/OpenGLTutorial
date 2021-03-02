#pragma once
#include "../dependencies/glm/glm.hpp"
#include "shader.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include <vector>
#include <fstream>
#include <iostream>

struct Material
{
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 emissive;
	float shininess;
};

class Mesh
{
public:
	Mesh(std::vector<Vertex>& vertices, uint64_t numVertices, std::vector<uint32_t>& indices, uint64_t numIndices, Material material, Shader* shader) {
		this->material = material;
		this->shader = shader;
		this->numIndices = numIndices;

		vertexBuffer = new VertexBuffer(vertices.data(), numVertices);
		indexBuffer = new IndexBuffer(indices.data(), numIndices, sizeof(uint32_t));

		diffuseLocation = glGetUniformLocation(shader->getShaderId(), "u_diffuse");
		specularLocation = glGetUniformLocation(shader->getShaderId(), "u_specular");
		emissiveLocation = glGetUniformLocation(shader->getShaderId(), "u_emissive");
		shininessLocation = glGetUniformLocation(shader->getShaderId(), "u_shininess");
	}
	~Mesh() {
		delete vertexBuffer;
		delete indexBuffer;
	}
	inline void render() {
		vertexBuffer->bind();
		indexBuffer->bind();
		glUniform3fv(diffuseLocation, 1, (float*)&material.diffuse);
		glUniform3fv(specularLocation, 1, (float*)&material.specular);
		glUniform3fv(emissiveLocation, 1, (float*)&material.emissive);
		glUniform1f(shininessLocation, material.shininess);
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	}
private:
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	Shader* shader;
	Material material;
	uint64_t numIndices = 0;
	int diffuseLocation;
	int specularLocation;
	int emissiveLocation;
	int shininessLocation;
};

class Model {
public:
	Model()
	{

	}

	void Init(const char* filename, Shader* shader) {
		std::ifstream input = std::ifstream(filename, std::ios::in | std::ios::binary);
		if (!input.is_open()) {
			std::cout << "Error reading model!" << std::endl;
		}

		uint64_t numMeshes;
		input.read((char*)&numMeshes, sizeof(uint64_t));
		for (uint64_t i = 0; i < numMeshes; i++)
		{
			std::vector<Vertex> vertices;
			uint64_t numVertices = 0;
			std::vector<uint32_t> indices;
			uint64_t numIndices = 0;
			Material material;

			input.read((char*)&material, sizeof(Material));

			input.read((char*)&numVertices, sizeof(uint64_t));
			input.read((char*)&numIndices, sizeof(uint64_t));
			for (uint64_t i = 0; i < numVertices; i++)
			{
				Vertex vertex;
				input.read((char*)&vertex.positon.x, sizeof(float));
				input.read((char*)&vertex.positon.y, sizeof(float));
				input.read((char*)&vertex.positon.z, sizeof(float));
				input.read((char*)&vertex.normal.x, sizeof(float));
				input.read((char*)&vertex.normal.y, sizeof(float));
				input.read((char*)&vertex.normal.z, sizeof(float));
				vertices.push_back(vertex);
			}
			for (uint64_t i = 0; i < numIndices; i++)
			{
				uint32_t index;
				input.read((char*)&index, sizeof(uint32_t));
				indices.push_back(index);
			}
			Mesh* mesh = new Mesh(vertices, numVertices, indices, numIndices , material, shader);
			meshes.push_back(mesh);
		}
	}
	void render() {
		for (Mesh* mesh : meshes) {
			mesh->render();
		}
	}

	~Model() {
		for (Mesh* mesh : meshes) {
			delete mesh;
		}
	}
private:
	std::vector<Mesh*> meshes;
};
