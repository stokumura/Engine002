#pragma once

#include <glad/gl.h>

#include "VertexAttribute.h"

#include "../Texture/Texture.h"
#include "../Texture/Sampler.h"
#include "../Shader/ShaderProgram.h"

class Mesh {
    private:
        VertexLayout layout;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<const Texture*> textures;

        GLuint VAO, VBO, EBO;
        unsigned int instances;

    public:
        Mesh();
        Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const std::vector<const Texture*> &textures, unsigned int instances = 0);
        ~Mesh();

        Mesh(const Mesh& other);
        Mesh& operator=(const Mesh& other);

        Mesh(Mesh &&other) noexcept;
        Mesh& operator=(Mesh &&ohter) noexcept;

        void Bind() const;
        static void Unbind();

        void Draw(ShaderProgram &shaderProgram, const Sampler &sampler) const;

    private:
        bool CreateBuffers(const Mesh *source);
        void SetupVertexArrayLayout();
        void Release();

        bool IsValid() const;
};
