#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <fstream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Position {
    float x, y, z;
};

std::vector<Position> positions;
std::vector<uint32_t> indices;

void processMesh(aiMesh* mesh, const aiScene* scene) {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Position vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        positions.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
}

void processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

char* getFilename(char* filename) {
    int len = strlen(filename);
    char* lastSlash = filename;
    for (int i = 0; i < len; i++) {
        if (filename[i] == '/' || filename[i] == '\\') {
            lastSlash = filename + i + 1;
        }
    }
    return lastSlash;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <modelfilename>" << std::endl;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(argv[argc - 1], aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE, !scene->mRootNode) {
        std::cout << "Error while loading model with assimp: " << importer.GetErrorString() << std::endl;
        return EXIT_FAILURE;
    }

    processNode(scene->mRootNode, scene);

    std::string filename = std::string(getFilename(argv[argc - 1]));
    std::string filenameWithoutExtension = filename.substr(0, filename.find_last_of('.'));
    // .bmf own file extension
    std::string outputFilename = filenameWithoutExtension + ".bmf";

    std::ofstream output(outputFilename, std::ios::out | std::ios::binary);
    std::cout << "Writing bmf file..." << std::endl;
    uint64_t numVertices = positions.size();
    uint64_t numIndices = indices.size();
    output.write((char*)&numVertices, sizeof(uint64_t));
    output.write((char*)&numIndices, sizeof(uint64_t));
    for (uint64_t i = 0; i < numVertices; i++)
    {
        output.write((char*)&positions[i].x, sizeof(float));
        output.write((char*)&positions[i].y, sizeof(float));
        output.write((char*)&positions[i].z, sizeof(float));
    }
    for (uint64_t i = 0; i < numIndices; i++)
    {
        output.write((char*)&indices[i], sizeof(uint32_t));
    }
    output.close();
    std::cout << "Finished!" << std::endl;
    return EXIT_SUCCESS;
}
