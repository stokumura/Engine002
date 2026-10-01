#pragma once

#include <vector>
#include <filesystem>
#include <memory>
#include "../Mesh/Mesh.h"
#include "../Texture/Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
    private:
        const VertexAttributeTypeLayout &attributes;
        std::vector<Mesh> meshes;
        std::vector<std::unique_ptr<Texture>> textures_loaded;
        std::filesystem::path directory;

    public:
        Model(const VertexAttributeTypeLayout &attributes, const std::filesystem::path &path);
        void Draw(ShaderProgram &shaderProgram, const Sampler &sampler) const;

    private:
        void loadModel(const std::filesystem::path &path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<const Texture*> loadMaterialTextures(aiMaterial *mat, aiTextureType type, TextureType internalType);
};
