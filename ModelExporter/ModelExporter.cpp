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

struct Material
{
    Position diffuse;
    Position specular;
    Position emissive;
    float shininess;
};

struct Mesh {
    std::vector<Position> positions;
    std::vector<Position> normals;
    std::vector<uint32_t> indices;
    Material material;
};

std::vector<Mesh> meshes;
std::vector<Material> materials;

void processMesh(aiMesh* mesh, const aiScene* scene) {
    Mesh m;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Position position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        m.positions.push_back(position);
        Position normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        m.normals.push_back(normal);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            m.indices.push_back(face.mIndices[j]);
        }
    }
    m.material = materials[mesh->mMaterialIndex];
    meshes.push_back(m);
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

void processMaterials(const aiScene* scene) {
    for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    {
        Material mat;
        aiMaterial* material = scene->mMaterials[i];

        aiColor3D diffuse(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse)) {
            std::cout << "No diffuse color." << std::endl;
        }
        mat.diffuse = { diffuse.r, diffuse.g, diffuse.b };
        aiColor3D specular(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_SPECULAR, specular)) {
            std::cout << "No specular color." << std::endl;
        }
        mat.specular = { specular.r, specular.g, specular.b };
        aiColor3D emissive(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive)) {
            std::cout << "No emissive color." << std::endl;
        }
        mat.emissive = { emissive.r, emissive.g, emissive.b };

        float shininess = 0.0f;
        if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS, shininess)) {
            std::cout << "No shininess." << std::endl;
        }
        mat.shininess = shininess;

        float shininessStrength = 0.0f;
        if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength)) {
            std::cout << "No shininess strength." << std::endl;
        }
        mat.specular.x *= shininessStrength;
        mat.specular.y *= shininessStrength;
        mat.specular.z *= shininessStrength;

        materials.push_back(mat);
    }
}


int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <modelfilename>" << std::endl;
    }

    Assimp::Importer importer;
    // aiProcess_PreTransformVertices not working in Debug mode
    const aiScene* scene = importer.ReadFile(argv[argc - 1], aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE, !scene->mRootNode) {
        std::cout << "Error while loading model with assimp: " << importer.GetErrorString() << std::endl;
        return EXIT_FAILURE;
    }

    processMaterials(scene);
    processNode(scene->mRootNode, scene);

    std::string filename = std::string(getFilename(argv[argc - 1]));
    std::string filenameWithoutExtension = filename.substr(0, filename.find_last_of('.'));
    // .bmf own file extension
    std::string outputFilename = filenameWithoutExtension + ".bmf";

    std::ofstream output(outputFilename, std::ios::out | std::ios::binary);
    std::cout << "Writing bmf file..." << std::endl;
    uint64_t numMeshes = meshes.size();
    output.write((char*)&numMeshes, sizeof(uint64_t));
    for (Mesh& mesh : meshes) {
        uint64_t numVertices = mesh.positions.size();
        uint64_t numIndices = mesh.indices.size();
        
        output.write((char*)&mesh.material, sizeof(Material));

        output.write((char*)&numVertices, sizeof(uint64_t));
        output.write((char*)&numIndices, sizeof(uint64_t));
        for (uint64_t i = 0; i < numVertices; i++)
        {
            output.write((char*)&mesh.positions[i].x, sizeof(float));
            output.write((char*)&mesh.positions[i].y, sizeof(float));
            output.write((char*)&mesh.positions[i].z, sizeof(float));

            output.write((char*)&mesh.normals[i].x, sizeof(float));
            output.write((char*)&mesh.normals[i].y, sizeof(float));
            output.write((char*)&mesh.normals[i].z, sizeof(float));
        }
        for (uint64_t i = 0; i < numIndices; i++)
        {
            output.write((char*)&mesh.indices[i], sizeof(uint32_t));
        }
    }
    output.close();
    std::cout << "Finished!" << std::endl;
    std::getchar();
    return EXIT_SUCCESS;
}
