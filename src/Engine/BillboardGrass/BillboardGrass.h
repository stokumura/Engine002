#pragma once

#include <glad/gl.h>
#include "../Texture/Texture.h"
#include "../Texture/Sampler.h"
#include "../Shader/ShaderProgram.h"
#include "../Primitive/Billboard.h"

struct DrawElementsIndirectCommand { GLuint count, instanceCount, firstIndex, baseVertex, baseInstance; };

class BillboardGrass {
    private:
        const int GX = 32, GY = 32;

        GLuint SSBO, IndirectBuf, VAO;

        const Texture &texture;

        Billboard billboard;
        Mesh mesh;

        unsigned int count;
    public:
        BillboardGrass(const Texture &texture);
        void Draw(const ShaderProgram &shaderProgram, const ShaderProgram &computeShaderProgram, const Sampler &sampler) const;
};
