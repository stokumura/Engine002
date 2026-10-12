#pragma once

#include <glad/gl.h>
#include "../Shader/ShaderProgram.h"
#include "../Model/Model.h"
#include "../BillboardGrass/BillboardGrass.h"

struct SingleLeafData {
    float height;
    glm::vec2 direction;
    glm::vec3 position;
    glm::vec2 pad;
};

class ModernGrass {
    private:
        const int GX = 64, GY = 64;

        GLuint SSBO, IndirectBuf, VAO;

        unsigned int count;

        const Model &model;

    public:
        ModernGrass(const Model &model);
        void Draw(const ShaderProgram &shaderProgram, const ShaderProgram &computeShaderProgram) const;
};
