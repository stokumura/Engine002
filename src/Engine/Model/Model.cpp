#include "Model.h"
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <algorithm>
#include "../../Utils/Logger/Logger.h"

Model::Model(const VertexAttributeTypeLayout& attributes, const std::filesystem::path &path) : attributes(attributes) {
    loadModel(path);
}

void Model::Draw(ShaderProgram &shaderProgram, const Sampler &sampler) const {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shaderProgram, sampler);
}

void Model::loadModel(const std::filesystem::path &path) {
    Assimp::Importer import;

    unsigned int postprocessFlags = aiProcess_Triangulate | aiProcess_FlipUVs;
    if(std::find(attributes.begin(), attributes.end(), VertexAttributeType::NORMAL) != attributes.end()) {
        postprocessFlags |= aiProcess_GenNormals;
    }
    if(std::find(attributes.begin(), attributes.end(), VertexAttributeType::TANGENT) != attributes.end() || std::find(attributes.begin(), attributes.end(), VertexAttributeType::BITANGENT) != attributes.end()) {
        postprocessFlags |= aiProcess_GenNormals;
        postprocessFlags |= aiProcess_CalcTangentSpace;
    }

    const aiScene *scene = import.ReadFile(path.string().c_str(), postprocessFlags);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG("ERROR::ASSIMP:: %s", import.GetErrorString());
        return;
    }
    directory = path.parent_path();
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<const Texture*> textures; 

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 position;
        glm::vec2 texCoords;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;

        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;

        if(mesh->mTextureCoords[0]) {
            texCoords.x = mesh->mTextureCoords[0][i].x;
            texCoords.y = mesh->mTextureCoords[0][i].y;
        }

        if(mesh->HasNormals()) {
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
        }

        if(mesh->HasTangentsAndBitangents()) {
            tangent.x = mesh->mTangents[i].x;
            tangent.y = mesh->mTangents[i].y;
            tangent.z = mesh->mTangents[i].z;

            bitangent.x = mesh->mBitangents[i].x;
            bitangent.y = mesh->mBitangents[i].y;
            bitangent.z = mesh->mBitangents[i].z;
        }

        for(auto attribType : attributes) {
            switch (attribType)
            {
                case VertexAttributeType::POSITION2D:
                    vertices.push_back(position[0]);
                    vertices.push_back(position[1]);
                    break;
                case VertexAttributeType::POSITION3D:
                    vertices.push_back(position[0]);
                    vertices.push_back(position[1]);
                    vertices.push_back(position[2]);
                    break;
                case VertexAttributeType::TEXCOORD:
                    vertices.push_back(texCoords[0]);
                    vertices.push_back(texCoords[1]);
                    break;
                case VertexAttributeType::NORMAL:
                    vertices.push_back(normal[0]);
                    vertices.push_back(normal[1]);
                    vertices.push_back(normal[2]);
                    break;
                case VertexAttributeType::TANGENT:
                    vertices.push_back(tangent[0]);
                    vertices.push_back(tangent[1]);
                    vertices.push_back(tangent[2]);
                    break;
                case VertexAttributeType::BITANGENT:
                    vertices.push_back(bitangent[0]);
                    vertices.push_back(bitangent[1]);
                    vertices.push_back(bitangent[2]);
                    break;
            }
        }
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<const Texture*> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<const Texture*> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    //std::vector<const Texture*> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::NORMAL);
    //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    std::vector<const Texture*> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, TextureType::HEIGHT);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    VertexLayout layout = VertexAttribute::GetVertexLayout(attributes);
    return Mesh(layout, vertices, indices, textures);
}

std::vector<const Texture*> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, TextureType internalType) {
    std::vector<const Texture*> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString texturePath;
        mat->GetTexture(type, i, &texturePath);
        std::filesystem::path textureAbsolutePath = directory / texturePath.C_Str();

        bool alreadyLoaded = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++) {
            if(std::strcmp(textures_loaded[j]->GetPath().c_str(), textureAbsolutePath.string().c_str()) == 0) {
                textures.push_back(textures_loaded[j].get());
                alreadyLoaded = true;
                break;
            }
        }
        if(!alreadyLoaded) {
            textures_loaded.push_back(std::make_unique<Texture>(textureAbsolutePath.string(), internalType, textures_loaded.size()));
        }
    }
    return textures;
}
